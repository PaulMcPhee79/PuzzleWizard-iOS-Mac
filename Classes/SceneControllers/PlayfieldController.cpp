

#ifdef DEBUG
    //#define IGNORE_PROGRESS
    //#define CM_CLR_CLOUD
    //#define TEST_REWARDS
#endif

#include "PlayfieldController.h"
#include "AppMacros.h"

#ifdef __APPLE__
    #include "../../../../CocosDenshion/include/SimpleAudioEngine.h"
#elif defined (__ANDROID__)
    #include "SimpleAudioEngine.h"
    #include <CMJniUtils.h>
	#include <Managers/LicenseManager.h>
#else
    #include "../CocosDenshion/include/SimpleAudioEngine.h"
#endif


#include <SceneViews/PlayfieldView.h>
#include <SceneControllers/MenuController.h>
#include <SceneControllers/GameController.h>
#include <Puzzle/View/Menu/MenuDialogManager.h>
#include <Utils/FpsSampler.h>
#include <Utils/CloudWrapper.h>
#include <Utils/Globals.h>
#include <Utils/Utils.h>
#include <Puzzle/Data/Puzzle.h>
#include <Puzzle/View/Playfield/TileDecoration.h>
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Puzzle/View/Playfield/PlayerPiece.h>
#include <Puzzle/View/Playfield/Effects/TileRotator.h>
#include <Puzzle/View/Playfield/Effects/TileConveyorBelt.h>
#include <Puzzle/View/Playfield/Effects/TileSwapper.h>
#include <Puzzle/View/Playfield/Effects/Shield.h>
#include <Puzzle/View/Playfield/Effects/Twinkle.h>
#include <Puzzle/View/Menu/LevelMenu.h>
#include <Puzzle/Controllers/PuzzleController.h>
#include <Puzzle/View/Menu/MenuBuilder.h>
#include <Managers/TouchManager.h>
#include <Utils/Jukebox.h>
#include <Utils/BridgingUtility.h>
#include <Utils/PWDebug.h>
#if CM_BETA
    #include <Testing/TFManager.h>
#endif
#include <Puzzle/Data/GameProgressEnums.h>
#include <Puzzle/Controllers/GameProgressController.h>
USING_NS_CC;

static const uint kCloudEvLoggedIn = 0x1UL;
static const uint kCloudEvLoggedOut = 0x2UL;
static const uint kCloudEvDataChanged = 0x4UL;
static const uint kCloudEvSettingsChanged = 0x8UL;
static const uint kCloudEvUnavailable = 0x10UL;

static int EV_TYPE_CLOUD_UNAVAILABLE_ALERT() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

PlayfieldController::PlayfieldController(void)
	:
    mIsContentLoaded(false),
    mHasSetupCompleted(false),
    mIgnoreNewPuzzleEvents(false),
    mHasProcessedSolvedPuzzle(false),
    mHasSampledFps(false),
    mHasReportedBuggedPuzzle(false),
    mIsCloudEnableEnqueued(false),
    mIsIdleTimerDisabled(true),
    mCloudEvents(0),
    mContentQueueCurrent(0),
    mContentQueuePeak(0),
	mMiscTimer(0.0f),
	mState(PlayfieldController::HIBERNATING),
	mPrevState(PlayfieldController::HIBERNATING),
	mPuzzleController(NULL),
	mView(NULL),
	mMenuController(NULL),
    mFpsSampler(NULL)
{
	mSceneKey = "Playfield";
    if (mJuggler)
        mJuggler->reserve(20);
}

PlayfieldController::~PlayfieldController(void)
{
    detachEventListeners();
    
    unsubscribeToInputUpdates(this);
    unsubscribeToInputUpdates(this, true);
    
	if (MenuBuilder::getScene() == this)
		MenuBuilder::setScene(NULL);
    if (mMenuController)
    {
        mMenuController->removeEventListener(MenuDialogManager::EV_TYPE_DID_CLOSE_LEVEL_COMPLETED_DIALOG(), this);
        CC_SAFE_RELEASE_NULL(mMenuController);
    }
	CC_SAFE_RELEASE_NULL(mView);
    
    if (mPuzzleController)
    {
        mPuzzleController->removeEventListener(PuzzleController::EV_TYPE_PUZZLE_DID_BEGIN(), this);
        mPuzzleController->removeEventListener(PuzzleController::EV_TYPE_PUZZLE_SOLVED_ANIMATION_COMPLETED(), this);
        mPuzzleController->removeEventListener(PuzzleController::EV_TYPE_LICENSE_LOCK_DID_TRIGGER(), this);
        mPuzzleController->removeEventListener(Puzzle::EV_TYPE_UNSOLVED_PUZZLE_WILL_SOLVE(), this);
        CC_SAFE_RELEASE_NULL(mPuzzleController);
    }
    
    Localizer::setScene(this, NULL);
    TextUtils::setScene(this, NULL);
}

void PlayfieldController::preSetupController(void)
{
    Localizer::setScene(this, this);
    TextUtils::setScene(this, this);
    setLocale(Localizer::getLaunchLocale());
    
    generateMipMaps();
    SceneController::preSetupController();
    MenuBuilder::setScene(this);
    mSpriteLayerManager = new SpriteLayerManager(mBaseSprite, CMGlobals::PFCAT_COUNT);
    
	mTouchManager = new TouchManager(CMGlobals::HUD, CCRectMake(0, 0,  getViewportWidth(), getViewportHeight()));
    mTouchManager->enable(false);
	mScene->addChild(mTouchManager);
}

void PlayfieldController::setupController(void)
{
	if (mView)
		return;
    
    subscribeToInputUpdates(this);
    subscribeToInputUpdates(this, true);

	mPuzzleController = new PuzzleController(this);
    mPuzzleController->addEventListener(PuzzleController::EV_TYPE_PUZZLE_DID_BEGIN(), this);
	mPuzzleController->addEventListener(PuzzleController::EV_TYPE_PUZZLE_SOLVED_ANIMATION_COMPLETED(), this);
	mPuzzleController->addEventListener(PuzzleController::EV_TYPE_LICENSE_LOCK_DID_TRIGGER(), this);
    mPuzzleController->addEventListener(Puzzle::EV_TYPE_UNSOLVED_PUZZLE_WILL_SOLVE(), this);
    
    mIgnoreNewPuzzleEvents = true;
	mPuzzleController->loadPuzzleByID(2); // Yellow Brick Road
    mIgnoreNewPuzzleEvents = false;

    // Hack: PlayfieldView needs a non-null PuzzleController for initialization.
	mView = new PlayfieldView(this);
	mView->setupView();

#ifdef __ANDROID__
	// MUST run prior to GPC()->load() and after PuzzleController setup
	LicenseManager::LM()->addEventListener(LicenseManager::EV_TYPE_LICENSE_DENIED(), this);
	LicenseManager::LM()->addEventListener(LicenseManager::EV_TYPE_LICENSE_APPROVED(), this);
	LicenseManager::LM()->addEventListener(LicenseManager::EV_TYPE_LICENSE_TRIAL_EXPIRED(), this);
	LicenseManager::LM()->addEventListener(LicenseManager::EV_TYPE_LICENSE_TRIAL_EXTENDED(), this);


#endif

#ifndef IGNORE_PROGRESS
	GameProgressController::GPC()->load();
#endif
	mMenuController = new MenuController(this);
    mMenuController->addEventListener(MenuDialogManager::EV_TYPE_DID_CLOSE_LEVEL_COMPLETED_DIALOG(), this);
	mMenuController->setupController();

	mMenuController->populateLevelMenu(mPuzzleController->getPuzzleOrganizer()->getLevels());
	setState(PlayfieldController::TITLE);
	setState(PlayfieldController::MENU);
    
    if (!getUserDefaultBool(CMSettings::B_SETTINGS_INITIALIZED, false))
    {
        // Initialize user defaults on first time played
        setUserDefault(CMSettings::B_SETTINGS_INITIALIZED, true);
#ifdef CHEEKY_MOBILE
        setUserDefault(CMSettings::B_SFX, true);
        setUserDefault(CMSettings::B_MUSIC, true);
#else
        setUserDefault(CMSettings::B_FULLSCREEN, true);
        setUserDefault(CMSettings::I_SFX, CMSettings::kDefaultSfxVolume);
        setUserDefault(CMSettings::I_MUSIC, CMSettings::kDefaultMusicVolume);
#endif
        
#ifdef __APPLE__
        setUserDefault(CMSettings::B_CLOUD_ENABLED, true);
#endif
        setUserDefault(CMSettings::B_COLOR_BLIND_MODE, false);
        flushUserDefaults();
        
        BridgingUtility::initCustomKeys();
    }
    
//#if CM_BETA
//    std::string betaName = getUserDefaultString(CMSettings::S_BETA_NAME);
//    if (betaName.empty())
//        TFManager::TFM()->showWelcomeView();
//    else
//        BridgingUtility::setUserIdentifier(betaName.c_str());
//#endif
    
    GameProgressController::GPC()->invalidateCaches();
    applyGameSettings();
    
#if DEBUG
    for (int i = 0; i < 12; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
    #ifdef CM_CLR_CLOUD
            GameProgressController::GPC()->setSolved(false, i, j); // Brand New
    #else
//            GameProgressController::GPC()->setSolved(false, i, j); // Brand New
//            GameProgressController::GPC()->setSolved(true, i, j); // Finished
//            GameProgressController::GPC()->setSolved(i == 0 || (i == 1 && j != 0), i, j); // Rating Unlocked (must begin puzzle then quit back to LevelMenu to prime pump)
//            GameProgressController::GPC()->setSolved(j > 3, i, j); // Level Unlocked
//            GameProgressController::GPC()->setSolved(i < 11 && j > 0, i, j); // Level Completed
//            GameProgressController::GPC()->setSolved(i < 11 && (i > 0 || j > 0), i, j); // Level Completed + Wizard Unlocked
//            GameProgressController::GPC()->setSolved(i > 0 || j > 0, i, j); // Puzzle Wizard
    #endif
        }
    }
#endif
    
#ifdef CHEEKY_MOBILE
    int interruptedPuzzleID = SceneController::getUserDefaultInt(CMSettings::I_INTERRUPTED_PUZZLE_ID, -1);
    if (interruptedPuzzleID != -1)
    {
        // Immediately unset the flag so that a crash caused by deserialization doesn't persist beyond a single app launch.
        setRestoringPreviousSession(true);
        SceneController::setUserDefault(CMSettings::I_INTERRUPTED_PUZZLE_ID, -1);
        SceneController::flushUserDefaults();
        
        muteSfx(true);
        if (mPuzzleController->loadPuzzleByID(interruptedPuzzleID))
        {
            if (mPuzzleController->deserializeCurrentPuzzle())
            {
                mMenuController->setState(MenuController::MENU_IN);
                mMenuController->setState(MenuController::TRANSITION_OUT);
                mMenuController->setState(MenuController::MENU_OUT);
                mMenuController->hideLevelMenuInstantaneously();
                setState(PlayfieldController::PLAYING);
            }
        }
        muteSfx(false);
        setRestoringPreviousSession(false);
    }
#endif
    
#ifdef __ANDROID__
	#if CM_LOW_POWER
    	CMJniUtils::setDirtyRenderMode(30);
    #endif
#endif

    beginFpsSample();

#ifdef __APPLE__
    #ifndef IGNORE_PROGRESS
    if (getUserDefaultBool(CMSettings::B_CLOUD_ENABLED, true))
        enableCloud(true);
    #endif
#endif
    
    mHasSetupCompleted = true;
}

void PlayfieldController::enqueueContent(ContentType type, std::string name)
{
    enqueueContent(type, 1, name);
}

void PlayfieldController::enqueueContent(ContentType type, int weighting, std::string name)
{
    mContentLoadQueue.push_back(ContentItem(type, weighting, 0, std::vector<std::string>(1, name)));
}

void PlayfieldController::beginContentLoad(void)
{
    if (isContentLoaded())
        return;
    
    ContentType type = IMAGE;
    mContentLoadQueue.reserve(100);
    enqueueContent(type, "bg");
    enqueueContent(type, "bg-menu");
    enqueueContent(type, "color-gradient");
    enqueueContent(type, "dialog-bg9");
    enqueueContent(type, "loading");
    enqueueContent(type, "menu-atlas");
    enqueueContent(type, "mirror-gradient");
    enqueueContent(type, "noise");
    enqueueContent(type, "plasma");
    enqueueContent(type, "playfield-atlas");
    enqueueContent(type, "refraction");
    enqueueContent(type, "shield-dome");
    enqueueContent(type, "shield-dome-btm");
    enqueueContent(type, "shield-dome-top");
    enqueueContent(type, "tile-shadow");
    enqueueContent(type, "tile-shadow-occ");
    enqueueContent(type, "sin2x-table");
    enqueueContent(type, "shield-stencil-0");
    enqueueContent(type, "shield-stencil-1");
    enqueueContent(type, "shield-stencil-2");
    enqueueContent(type, "shield-stencil-3");
    enqueueContent(type, "shield-stencil-4");
    enqueueContent(type, "sparkle-gradient");
    enqueueContent(type, "wizard-idle-atlas");
    enqueueContent(type, "wizard-walk-atlas");
#ifdef CHEEKY_DESKTOP
    enqueueContent(type, 0, "locales-atlas");
#endif
    
    type = ATLAS;
    {
        const char* names[] = { "menu-atlas.plist", "menu-atlas" };
        mContentLoadQueue.push_back(ContentItem(type, 1, 0, CMUtils::makeStrVector(names)));
    }
    {
        const char* names[] = { "playfield-atlas.plist", "playfield-atlas" };
        mContentLoadQueue.push_back(ContentItem(type, 1, 0, CMUtils::makeStrVector(names)));
    }
    {
        const char* names[] = { "wizard-idle-atlas.plist", "wizard-idle-atlas" };
        mContentLoadQueue.push_back(ContentItem(type, 1, 0, CMUtils::makeStrVector(names)));
    }
    {
        const char* names[] = { "wizard-walk-atlas.plist", "wizard-walk-atlas" };
        mContentLoadQueue.push_back(ContentItem(type, 1, 0, CMUtils::makeStrVector(names)));
    }
#ifdef CHEEKY_DESKTOP
    {
        const char* names[] = { "locales-atlas.plist", "locales-atlas" };
        mContentLoadQueue.push_back(ContentItem(type, 1, 0, CMUtils::makeStrVector(names)));
    }
#endif
    
    type = FONT;
    {
        {
            enqueueContent(IMAGE, "fonts/IQ-40_0");
            const char* names[] = { "fonts/IQ-40" };
            mContentLoadQueue.push_back(ContentItem(type, 1, 40, CMUtils::makeStrVector(names)));
        }
        
        {
            enqueueContent(IMAGE, "fonts/IQ-24_0");
            const char* names[] = { "fonts/IQ-24" };
            mContentLoadQueue.push_back(ContentItem(type, 1, 24, CMUtils::makeStrVector(names)));
        }
    }
    
    type = AUDIO;
    enqueueContent(type, "button");
    if (MODE_8x6)
    {
        enqueueContent(type, "cbelt-horiz_8x6");
        enqueueContent(type, "cbelt-vert_8x6");
    }
    else
    {
        enqueueContent(type, "cbelt-horiz");
        enqueueContent(type, "cbelt-vert");
    }
    enqueueContent(type, "color-arrow-medium");
    enqueueContent(type, "color-arrow-short");
    enqueueContent(type, "color-arrow");
    enqueueContent(type, "color-flood-medium");
    enqueueContent(type, "color-flood-short");
    enqueueContent(type, "color-flood");
    enqueueContent(type, "color-magic");
    enqueueContent(type, "color-swap");
    enqueueContent(type, "color-swirl-medium");
    enqueueContent(type, "color-swirl-short");
    enqueueContent(type, "color-swirl");
    enqueueContent(type, "crowd-cheer");
    enqueueContent(type, "error");
    enqueueContent(type, "level-unlocked");
    enqueueContent(type, "locked");
    enqueueContent(type, "mirrored-self");
    enqueueContent(type, "player-teleport");
    enqueueContent(type, "reset");
    enqueueContent(type, "rotate");
    enqueueContent(type, "solved");
    enqueueContent(type, "solved-short");
    enqueueContent(type, "tile-shield-activate");
    enqueueContent(type, "tile-shield-deactivate");
    enqueueContent(type, "tile-swap");
    enqueueContent(type, "unlocked");
    
    enqueueContent(PRE_SETUP, 10, "preSetupController");
    enqueueContent(PROGRAMS, 5, "setupPrograms");
    enqueueContent(REUSABLES, 5, "setupReusables");
    enqueueContent(SETUP, 5, "setupController");
    for (int i = 0; i < 10; ++i)
        enqueueContent(PAUSE, 0, "frameSkip");
    
    mContentLoadQueue = ContentQueue(mContentLoadQueue.rbegin(), mContentLoadQueue.rend());
    
    mContentQueuePeak = 0;
    for (ContentQueue::iterator it = mContentLoadQueue.begin(); it != mContentLoadQueue.end(); ++it)
        mContentQueuePeak += (*it).weighting;
    mContentQueueCurrent = mContentQueuePeak;
    
    CCLog("Enqueued %d content items.", mContentQueuePeak);
}

void PlayfieldController::loadNextContent(void)
{
    if (!mContentLoadQueue.empty())
    {
        struct ContentItem item = mContentLoadQueue.back();
        switch (item.type) {
            case IMAGE:
                CCTextureCache::sharedTextureCache()->addImage(item.names[0].append(CM_IMG_EXT).c_str());
                CCLog("Loaded image: %s", item.names[0].c_str());
                break;
            case ATLAS:
                CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(item.names[0].c_str(), textureByName(item.names[1].c_str()));
                CCLog("Loaded atlas: %s", item.names[0].c_str());
                break;
            case FONT:
                addFontName(item.names[0].append(CM_FONT_EXT).c_str(), item.fontSize);
                CCLog("Loaded font: %s", item.names[0].c_str());
                break;
            case AUDIO:
                CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect(item.names[0].append(CM_SND_EXT).c_str());
                CCLog("Loaded audio: %s", item.names[0].c_str());
                break;
            case PRE_SETUP:
                preSetupController();
                CCLog("Content load pre-setup");
                break;
            case PROGRAMS:
                setupPrograms(false);
                CCLog("Content load programs");
                break;
            case REUSABLES:
                setupReusables();
                CCLog("Content load reusables");
                break;
            case SETUP:
                setupController();
                CCLog("Content load setup");
                break;
            case PAUSE:
                CCLog("Content load pause");
                break;
            default:
                CCAssert(false, "PlayfieldController::loadNextContent - invalid ContentType in load queue.");
                break;
        }
        
        mContentQueueCurrent -= item.weighting;
        mContentLoadQueue.pop_back();
    }
    else
    {
        ContentQueue().swap(mContentLoadQueue); // shrink it
        mIsContentLoaded = true;
        CCLog("Content load completed");
    }
}

bool PlayfieldController::isContentLoaded(void)
{
    return mIsContentLoaded;
}

float PlayfieldController::getContentLoadProgress(void)
{
    return mContentQueuePeak <= 0 ? 0 : 1.0f - mContentQueueCurrent / (float)mContentQueuePeak;
}

void PlayfieldController::generateMipMaps(void)
{
#ifdef CHEEKY_MOBILE
	CCLog("Generating mipmaps...");

    {
        const int kMipMapCount = 4;
        const char* texNames[kMipMapCount] =
        {
            "bg-menu",
            "playfield-atlas",
            "fonts/IQ-40_0",
            "fonts/IQ-24_0"
        };

        //ccTexParams params = {GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_REPEAT,GL_REPEAT}; // Most Expensive
        //ccTexParams params = {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_REPEAT,GL_REPEAT}; // Expensive
        //ccTexParams params = {GL_LINEAR_MIPMAP_NEAREST,GL_NEAREST,GL_REPEAT,GL_REPEAT}; // Cheap

        ccTexParams texParams[kMipMapCount] =
        {
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_REPEAT,GL_REPEAT},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}
        };


        for (int i = 0; i < kMipMapCount; ++i)
        {
            CCTexture2D* tex = textureByName(texNames[i]);
            if (tex)
            {
                tex->setTexParameters(&texParams[i]);
                tex->generateMipmap();
            }
        }
    }
    
    
#elif defined(CHEEKY_DESKTOP)
    CCLog("Generating mipmaps...");
    
    {
        const int kMipMapCount = 9;
        const char* texNames[kMipMapCount] =
        {
            "bg",
            "bg-menu",
            "locales-atlas",
            "menu-atlas",
            "playfield-atlas",
            "wizard-idle-atlas",
            "wizard-walk-atlas",
            "fonts/IQ-40_0",
            "fonts/IQ-24_0"
        };
        
        //ccTexParams params = {GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_REPEAT,GL_REPEAT}; // Most Expensive
        //ccTexParams params = {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_REPEAT,GL_REPEAT}; // Expensive
        //ccTexParams params = {GL_LINEAR_MIPMAP_NEAREST,GL_NEAREST,GL_REPEAT,GL_REPEAT}; // Cheap
        
        ccTexParams texParams[kMipMapCount] =
        {
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_REPEAT,GL_REPEAT},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_REPEAT,GL_REPEAT},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE},
            {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}
        };
        
        
        for (int i = 0; i < kMipMapCount; ++i)
        {
            CCTexture2D* tex = textureByName(texNames[i]);
            if (tex)
            {
                tex->setTexParameters(&texParams[i]);
                tex->generateMipmap();
            }
        }
    }
    
#endif
}

void PlayfieldController::applicationDidEnterBackground(void)
{
    SceneController::applicationDidEnterBackground();
    finishFpsSample();
    
    CloudWrapper::applicationWillResignActive();
    serializeCurrentPuzzle();
}

void PlayfieldController::applicationWillEnterForeground(void)
{
    SceneController::applicationWillEnterForeground();
    
    CloudWrapper::applicationDidBecomeActive();
    
#ifdef CHEEKY_MOBILE
    if (mHasSetupCompleted)
    {
        if (SceneController::getUserDefaultInt(CMSettings::I_INTERRUPTED_PUZZLE_ID, -1) != -1)
        {
            SceneController::setUserDefault(CMSettings::I_INTERRUPTED_PUZZLE_ID, -1);
            SceneController::flushUserDefaults();
        }
    }
#endif

#ifdef __ANDROID__
    // Hint towards a license check in case wifi was enabled since the app lost focus
    if (mHasSetupCompleted)
    	LicenseManager::LM()->setLicenseCheckHint();
#endif
}

void PlayfieldController::applicationWillTerminate(void)
{
    serializeCurrentPuzzle();
}

void PlayfieldController::splashViewDidHide(void)
{
    SceneController::splashViewDidHide();
    if (mTouchManager)
        mTouchManager->enable(true);
#if CM_BETA
    TFManager::TFM()->applyPendingState();
#endif
}

void PlayfieldController::beginFpsSample(void)
{
#if 0
    mHasSampledFps = true;
#else
    if (mFpsSampler)
        return;
    
    mFpsSampler = FpsSampler::createWithCategory(CMGlobals::SUB_BG, this, false);
    addProp(mFpsSampler);
    
    float x = getViewWidth() / 4, y = getViewHeight() / 4;
    for (int i = 0; i < 1; ++i)
    {
        Shield* shield = Shield::createWith(CMGlobals::BG, 1000 + i, 0, ccp(x, y));
        mFpsSampler->addSample(shield);
        shield->deploy(false);
        
        //CCLog("mFpsSampler shield address:%p", shield);
        
        x += getViewWidth() / 2;
        if (x > getViewWidth())
        {
            x = getViewWidth() / 4;
            y += getViewHeight() / 2;
            if (y > getViewHeight())
                y = getViewHeight() / 4;
        }
    }
        
//        mFpsSampler->beginSampling(2.0f, 2.25f); // 2.25s delay so that splash screen is hidden (splash screen lags FPS).
    mFpsSampler->beginSampling(1.0f, 0); // 2.25s delay so that splash screen is hidden (splash screen lags FPS).
    addToJuggler(mFpsSampler);
#endif
}

void PlayfieldController::finishFpsSample(void)
{
//#ifdef CHEEKY_MOBILE
    if (mFpsSampler)
    {
        removeFromJuggler(mFpsSampler);
        removeProp(mFpsSampler);
        CC_SAFE_RELEASE_NULL(mFpsSampler);
    }
//#endif
}

void PlayfieldController::setupReusables(void)
{
	TileDecoration::setupReusables();
	TilePiece::setupReusables();
	Twinkle::setupReusables();
	PlayerPiece::setupReusables();
	TileRotator::setupReusables();
	TileConveyorBelt::setupReusables();
	TileSwapper::setupReusables();
	Shield::setupReusables();
    
    CCPoolManager::sharedPoolManager()->pop();
}

void PlayfieldController::setupPrograms(bool reload)
{
	// Refraction
	CCGLProgram *glProgram = 0;
	if (reload)
	{
		glProgram = CCShaderCache::sharedShaderCache()->programForKey("Refraction");
		glProgram->reset();
	}
	else
	{
		glProgram = new CCGLProgram(); glProgram->autorelease();
		CCShaderCache::sharedShaderCache()->addProgram(glProgram, "Refraction");
	}

	glProgram->initWithVertexShaderFilename("shaders/Refraction.vsh", "shaders/Refraction.fsh");
	glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
	glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
	glProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
	glProgram->addAttribute(kCCAttributeNameShaderCoord, kCCVertexAttrib_ShaderCoords);
	glProgram->link();
	glProgram->updateUniforms();

	GLuint program = glProgram->getProgram();
	GLuint textureUnit = 1;
	int displaceUniformLoc = glGetUniformLocation(program,"u_displacementTex");
	glUniform1i(displaceUniformLoc, textureUnit);

	CCTexture2D* displaceTex = textureByName("refraction");
    //ccTexParams params = {GL_LINEAR,GL_LINEAR,GL_REPEAT,GL_REPEAT}; // Expensive
	ccTexParams params = {GL_NEAREST,GL_NEAREST,GL_REPEAT,GL_REPEAT}; // Cheap
	displaceTex->setTexParameters(&params);
    
	setTextureUnit(displaceTex->getName(), textureUnit);
	setUniformLocation("u_scrollR", glGetUniformLocation(program, "u_scroll"));
	setUniformLocation("u_displacementScrollR", glGetUniformLocation(program, "u_displacementScroll"));
	//setUniformLocation("u_displacementFactorR", glGetUniformLocation(program, "u_displacementFactor"));

	// Color Gradient
	if (reload)
	{
		glProgram = CCShaderCache::sharedShaderCache()->programForKey("ColorGradient");
		glProgram->reset();
	}
	else
	{
		glProgram = new CCGLProgram(); glProgram->autorelease();
		CCShaderCache::sharedShaderCache()->addProgram(glProgram, "ColorGradient");
	}

	glProgram->initWithVertexShaderFilename("shaders/Default.vsh", "shaders/ColorGradient.fsh");
	glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
	glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
	glProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
	glProgram->link();
	glProgram->updateUniforms();

	program = glProgram->getProgram();
	textureUnit = 2;
	int colorGradientUniformLoc = glGetUniformLocation(program,"u_gradientTex");
	glUniform1i(colorGradientUniformLoc, textureUnit);

	CCTexture2D* colorGradientTex = textureByName("color-gradient");
	colorGradientTex->setTexParameters(&params);

	setTextureUnit(colorGradientTex->getName(), textureUnit);
	setUniformLocation("u_gradientCoordCG", glGetUniformLocation(program, "u_gradientCoord"));
    
	// Sparkle Gradient
	if (reload)
	{
		glProgram = CCShaderCache::sharedShaderCache()->programForKey("SparkleGradient");
		glProgram->reset();
	}
	else
	{
		glProgram = new CCGLProgram(); glProgram->autorelease();
		CCShaderCache::sharedShaderCache()->addProgram(glProgram, "SparkleGradient");
	}

	glProgram->initWithVertexShaderFilename("shaders/Default.vsh", "shaders/Sparkle.fsh");
	glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
	glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
	glProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
	glProgram->addAttribute(kCCAttributeNameShaderCoord, kCCVertexAttrib_ShaderCoords);
	glProgram->link();
	glProgram->updateUniforms();

	program = glProgram->getProgram();
	textureUnit = 2; // Re-use Color Gradient slot.
	int sparkleGradientUniformLoc = glGetUniformLocation(program,"u_gradientTex");
	glUniform1i(sparkleGradientUniformLoc, textureUnit);

	CCTexture2D* sparkleGradientTex = textureByName("sparkle-gradient");
	sparkleGradientTex->setTexParameters(&params);

	setTextureUnit(sparkleGradientTex->getName(), textureUnit);
	setUniformLocation("u_gradientCoordSG", glGetUniformLocation(program, "u_gradientCoord"));

	// Mirror Image
	if (reload)
	{
		glProgram = CCShaderCache::sharedShaderCache()->programForKey("MirrorImage");
		glProgram->reset();
	}
	else
	{
		glProgram = new CCGLProgram(); glProgram->autorelease();
		CCShaderCache::sharedShaderCache()->addProgram(glProgram, "MirrorImage");
	}

	glProgram->initWithVertexShaderFilename("shaders/Default.vsh", "shaders/MirrorImage.fsh");
	glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
	glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
	glProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
	glProgram->link();
	glProgram->updateUniforms();

    program = glProgram->getProgram();
	textureUnit = 2; // Re-use Color Gradient texture unit (these two will never be active concurrently)
	int mirrorUniformLoc = glGetUniformLocation(program,"u_gradientTex");
	glUniform1i(mirrorUniformLoc, textureUnit);
    
	CCTexture2D* mirrorTex = textureByName("mirror-gradient");
	mirrorTex->setTexParameters(&params);
    
	setTextureUnit(mirrorTex->getName(), textureUnit);
	setUniformLocation("u_gradientCoordMI", glGetUniformLocation(program, "u_gradientCoord"));

	// Dissolve
	if (reload)
	{
		glProgram = CCShaderCache::sharedShaderCache()->programForKey("Dissolve");
		glProgram->reset();
	}
	else
	{
		glProgram = new CCGLProgram(); glProgram->autorelease();
		CCShaderCache::sharedShaderCache()->addProgram(glProgram, "Dissolve");
	}

	glProgram->initWithVertexShaderFilename("shaders/Default.vsh", "shaders/Dissolve.fsh");
	glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
	glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
	glProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
	glProgram->link();
	glProgram->updateUniforms();

	program = glProgram->getProgram();
	textureUnit = 3;
	int dissolveUniformLoc = glGetUniformLocation(glProgram->getProgram(),"u_noiseTex");
	glUniform1i(dissolveUniformLoc, textureUnit);

	CCTexture2D* noiseTex = textureByName("noise");
    ccTexParams noiseParams = {GL_LINEAR_MIPMAP_NEAREST,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE};
	noiseTex->setTexParameters(&noiseParams);
    noiseTex->generateMipmap();

	setTextureUnit(noiseTex->getName(), textureUnit);
	setUniformLocation("u_thresholdD", glGetUniformLocation(program, "u_threshold"));

	// Shield
	if (reload)
	{
		glProgram = CCShaderCache::sharedShaderCache()->programForKey("Shield");
		glProgram->reset();
	}
	else
	{
		glProgram = new CCGLProgram(); glProgram->autorelease();
		CCShaderCache::sharedShaderCache()->addProgram(glProgram, "Shield");
	}

	glProgram->initWithVertexShaderFilename("shaders/Shield.vsh", "shaders/Shield.fsh");
	glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
	glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
	glProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
	glProgram->link();
	glProgram->updateUniforms();

	program = glProgram->getProgram();
	textureUnit = 4;
	int plasmaUniformLoc = glGetUniformLocation(program,"u_plasmaTex");
	glUniform1i(plasmaUniformLoc, textureUnit);
    
    CCTexture2D* plasmaTex = textureByName("plasma");
	plasmaTex->setTexParameters(&params);
	setTextureUnit(plasmaTex->getName(), textureUnit);
    
    textureUnit = 5;
	int sin2xUniformLoc = glGetUniformLocation(program,"u_sin2xTable");
	glUniform1i(sin2xUniformLoc, textureUnit);
    
    CCTexture2D* sin2xTex = textureByName("sin2x-table");
	sin2xTex->setTexParameters(&params);
	setTextureUnit(sin2xTex->getName(), textureUnit);
    
    textureUnit = 6;
    int shieldStencilUniformLoc0 = glGetUniformLocation(program,"u_stencilTex0");
    glUniform1i(shieldStencilUniformLoc0, textureUnit);
    
    textureUnit = 7;
    int shieldStencilUniformLoc1 = glGetUniformLocation(program,"u_stencilTex1");
    glUniform1i(shieldStencilUniformLoc1, textureUnit);
    
    glUniform1f(glGetUniformLocation(program, "u_shieldAlpha"), 0.65f);
	setUniformLocation("u_displacementScrollS", glGetUniformLocation(program, "u_displacementScroll"));
	setUniformLocation("u_stencilRotation0S", glGetUniformLocation(program, "u_stencilRotation0"));
    setUniformLocation("u_stencilRotation1S", glGetUniformLocation(program, "u_stencilRotation1"));
    
	// Tile shadow
    if (reload)
    {
    	glProgram = CCShaderCache::sharedShaderCache()->programForKey("TileShadow");
    	glProgram->reset();
    }
    else
    {
    	glProgram = new CCGLProgram(); glProgram->autorelease();
    	CCShaderCache::sharedShaderCache()->addProgram(glProgram, "TileShadow");
    }

	glProgram->initWithVertexShaderFilename("shaders/TileShadow.vsh", "shaders/TileShadow.fsh");
	glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
	glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
	glProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
	glProgram->link();
	glProgram->updateUniforms();

	program = glProgram->getProgram();
	setUniformLocation("u_occRegionTS", glGetUniformLocation(program, "u_occRegion"));

	textureUnit = 3; // Re-use dissolve slot. This would be slot 8, but there are only 8 slots available (0-7).
    int tileShadowOccUniformLoc = glGetUniformLocation(program,"u_occTexture");
    glUniform1i(tileShadowOccUniformLoc, textureUnit);

    ccTexParams occParams = {GL_NEAREST,GL_NEAREST,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE};
	CCTexture2D* shadowOccTex = textureByName("tile-shadow-occ");
    sin2xTex->setTexParameters(&occParams);
	setTextureUnit(shadowOccTex->getName(), textureUnit);
    
    // Blur
    if (reload)
	{
		glProgram = CCShaderCache::sharedShaderCache()->programForKey("Blur");
		glProgram->reset();
	}
	else
	{
		glProgram = new CCGLProgram(); glProgram->autorelease();
		CCShaderCache::sharedShaderCache()->addProgram(glProgram, "Blur");
	}
    
    glProgram->initWithVertexShaderFilename("shaders/Default.vsh", "shaders/Blur.fsh");
	glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
	glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
	glProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
	glProgram->link();
	glProgram->updateUniforms();
    
	program = glProgram->getProgram();
    setUniformLocation("u_blurSize", glGetUniformLocation(program, "u_blurSize"));
    
//    CCSize s = getTexture()->getContentSizeInPixels();
//    blur_ = ccp(1/s.width, 1/s.height);
//    blur_ = ccpMult(blur_,f);
    
	if (reload)
	{
		// Colored Quad
		glProgram = CCShaderCache::sharedShaderCache()->programForKey("ColoredQuad");
		glProgram->reset();
		glProgram->initWithVertexShaderFilename("shaders/ColoredQuad.vsh", "shaders/ColoredQuad.fsh");

		glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
		glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
		glProgram->link();
		glProgram->updateUniforms();
	}
	else
	{
#if defined(__APPLE__) && defined(CHEEKY_MOBILE)
		// Colored Quad
		glProgram = new CCGLProgram(); glProgram->autorelease();
		glProgram->initWithVertexShaderFilename("shaders/ColoredQuad.vsh", "shaders/ColoredQuad.fsh");
		CCShaderCache::sharedShaderCache()->addProgram(glProgram, "ColoredQuad");

		glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
		glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
		glProgram->link();
		glProgram->updateUniforms();
#endif
	}
}

void PlayfieldController::willGainSceneFocus(void)
{
    SceneController::willGainSceneFocus();
}

void PlayfieldController::willLoseSceneFocus(void)
{
    SceneController::willLoseSceneFocus();
}

void PlayfieldController::attachEventListeners(void)
{
    CloudWrapper::addEventListener(CloudWrapper::getEvTypeLoggedIn(), this);
    CloudWrapper::addEventListener(CloudWrapper::getEvTypeLoggedOut(), this);
    CloudWrapper::addEventListener(CloudWrapper::getEvTypeDataChanged(), this);
    CloudWrapper::addEventListener(CloudWrapper::getEvTypeSettingsChanged(), this);
    CloudWrapper::addEventListener(CloudWrapper::getEvTypeUnavailable(), this);
}

void PlayfieldController::detachEventListeners(void)
{
    CloudWrapper::removeEventListener(CloudWrapper::getEvTypeLoggedIn(), this);
    CloudWrapper::removeEventListener(CloudWrapper::getEvTypeLoggedOut(), this);
    CloudWrapper::removeEventListener(CloudWrapper::getEvTypeDataChanged(), this);
    CloudWrapper::removeEventListener(CloudWrapper::getEvTypeSettingsChanged(), this);
    CloudWrapper::removeEventListener(CloudWrapper::getEvTypeUnavailable(), this);

#ifdef __ANDROID__
    LicenseManager::LM()->removeEventListener(LicenseManager::EV_TYPE_LICENSE_DENIED(), this);
    LicenseManager::LM()->removeEventListener(LicenseManager::EV_TYPE_LICENSE_APPROVED(), this);
    LicenseManager::LM()->removeEventListener(LicenseManager::EV_TYPE_LICENSE_TRIAL_EXPIRED(), this);
    LicenseManager::LM()->removeEventListener(LicenseManager::EV_TYPE_LICENSE_TRIAL_EXTENDED(), this);
#endif
}

void PlayfieldController::applyGameSettings(void)
{
    SceneController::applyGameSettings();
    
    if (mMenuController)
        mMenuController->applyGameSettings();
    
    if (getUserDefaultBool(CMSettings::B_COLOR_BLIND_MODE))
        Puzzles::setColorScheme(Puzzles::CS_COLOR_BLIND);
}

void PlayfieldController::setCloudEvent(uint evCode, bool value)
{
    if (value)
        mCloudEvents |= evCode;
    else
        mCloudEvents &=~ evCode;
}

void PlayfieldController::clearCloudEvents(void)
{
    mCloudEvents = 0;
}

void PlayfieldController::processCloudEvents(void)
{
#ifdef __APPLE__
	static int s_CloudDataUpdateCount = 0;
	//static int s_CloudSettingsUpdateCount = 0;

    if (mCloudEvents == 0)
        return;
    
    if ((mCloudEvents & kCloudEvLoggedIn) == kCloudEvLoggedIn || (mCloudEvents & kCloudEvDataChanged) == kCloudEvDataChanged)
    {
        ulong size = 0;
        const void* data = CloudWrapper::load(size);
        if (data == NULL)
        {
            // Cloud initialized - send local save data to cloud.
            data = GameProgressController::GPC()->exportData(size);
            if (data)
                CloudWrapper::save(data, size);
        }
        else
        {
	#if DEBUG && defined(CM_CLR_CLOUD)
        if ((mCloudEvents & kCloudEvLoggedIn) == kCloudEvLoggedIn)
        {
            GameProgressController::GPC()->forceSave();
            
            ulong gpSize = 0;
            const u8* gpData = GameProgressController::GPC()->exportData(gpSize);
            if (gpData && gpSize > 0)
            {
                CloudWrapper::save(gpData, gpSize);
                CCLog("iCloud active: Clear data succeeded.");
            }
        } /* else do nothing */
	#else
            UpgradeStatus status = GameProgressController::GPC()->upgradeToData((const u8*)data, size);
            
            // Only re-save to cloud if there were differences in the data. Otherwise we will ping-pong updates back and forth continuously.
            
            if (status == US_LOCAL_UPGRADE || status == US_FULL_UPGRADE)
            {
                refreshAfterProgressChanged();
                GameProgressController::GPC()->save();
            }
            
            if (status == US_CLOUD_UPGRADE || status == US_FULL_UPGRADE)
            {
                data = GameProgressController::GPC()->exportData(size);
                if (data)
                    CloudWrapper::save(data, size);
            }
	#endif
        }
        
        if ((mCloudEvents & kCloudEvLoggedIn) == kCloudEvLoggedIn)
            cloudInitDidComplete(true);

        CCLog("Cloud data update %d.", ++s_CloudDataUpdateCount);
    }
    else if ((mCloudEvents & kCloudEvUnavailable) == kCloudEvUnavailable)
    {
        cloudInitDidComplete(false);
    }
    
    clearCloudEvents();
#endif
}

void PlayfieldController::refreshAfterProgressChanged(void)
{
    mMenuController->refreshLevelMenu();
}

void PlayfieldController::enableGodMode(bool enable)
{
//#if DEBUG && GOD_MODE
	if (mMenuController)
		mMenuController->enableGodMode(enable);
    GameProgressController::GPC()->setUnlockedAll(enable);
//#endif
}

void PlayfieldController::reportBuggedPuzzle()
{
#if CM_BETA
    if (!mHasReportedBuggedPuzzle)
    {
        mHasReportedBuggedPuzzle = true;
        
        PuzzleController* puzzleController = getPuzzleController();
        if (puzzleController)
        {
            Puzzle* puzzle = puzzleController->getPuzzle();
            if (puzzle)
            {
                std::string puzzleState = puzzle->getCurrentPuzzleSerialized();
                if (!puzzleState.empty())
                {
                    //CCLog("%s", puzzleState.c_str());
                    EvLogParamKV params;
                    for (int i = 0, pos = 0, len = puzzleState.size(); pos < len; i++, pos += 255)
                    {
                        std::string key = CMUtils::strConcatVal("PuzzleState_", i);
                        params[key] = puzzleState.substr(pos, 255); // 255 is Flurry's max length event parameter
                    }
                    BridgingUtility::logEvent(BridgingUtility::EV_NAME_BETA_PUZZLE_BUGGED, params);
                }
            }
        }
    }
    
    if (mMenuController)
        mMenuController->showMenuDialog("BetaReportBugThanks");
#endif
}

void PlayfieldController::showBetaIQRatingDialog()
{
#if CM_BETA
    PuzzleController* puzzleController = getPuzzleController();
    if (puzzleController && mMenuController)
    {
        Puzzle* puzzle = puzzleController->getPuzzle();
        if (puzzle && puzzle->getName())
            mMenuController->showBetaIQRatingDialog(puzzle->getIQ(), puzzle->getName());
    }
#endif
}

void PlayfieldController::setColorScheme(Puzzles::ColorScheme value)
{
    SceneController::setColorScheme(value);
    
    if (mMenuController)
        mMenuController->refreshColorScheme();
    if (getPuzzleController())
        getPuzzleController()->refreshColorScheme();
}

void PlayfieldController::enableLowPowerMode(bool enable)
{
    SceneController::enableLowPowerMode(enable);
    
    if (mView)
		mView->enableLowPowerMode(enable);
    if (mMenuController)
        mMenuController->refreshLowPowerMode();
}

void PlayfieldController::enableMenuMode(bool enable)
{
	SceneController::enableMenuMode(enable);

	if (mView)
		mView->enableMenuMode(enable);
	if (getPuzzleController())
		getPuzzleController()->enableMenuMode(enable);
}

void PlayfieldController::showMenuDialog(const char* key)
{
	SceneController::showMenuDialog(key);

	if (mMenuController)
		mMenuController->showMenuDialog(key);
}

void PlayfieldController::showEscDialog(void)
{
	if (mMenuController)
		mMenuController->showEscDialog(getState());
}

void PlayfieldController::hideEscDialog(void)
{
    SceneController::hideEscDialog();
    
	if (mMenuController)
		mMenuController->hideEscDialog(getState());
}

void PlayfieldController::setState(PfState value)
{
	if (value == mState)
        return;
    
    mPrevState = mState;

    // Clean up previous state
    switch (mPrevState)
    {
        case PlayfieldController::HIBERNATING:
            break;
        case PlayfieldController::TITLE:
            popFocusState(CMInputs::FOCUS_STATE_TITLE);
            break;
        case PlayfieldController::MENU:
            break;
        case PlayfieldController::PLAYING:
            popFocusState(CMInputs::FOCUS_STATE_PF_PLAYFIELD);
            
            if (value == PlayfieldController::MENU)
                processMilestoneDialogs();
            break;
    }

    // Apply new state
    mState = value;
    
    switch (mState)
    {
		case PlayfieldController::HIBERNATING:
            break;
		case PlayfieldController::TITLE:
            pushFocusState(CMInputs::FOCUS_STATE_TITLE);
			mMenuController->setState(MenuController::MENU_TITLE);
            mMiscTimer = 4.5f;
            break;
		case PlayfieldController::MENU:
            // NOTE(1): TFManager::TFM() must be NULL here else OpenGL gets angry when the UIView states become invalid
#if CM_BETA
//            if (mPrevState == PlayfieldController::TITLE)
//                TFManager::TFM_ASSERT_NULL();
#endif
            
			mMenuController->setState(MenuController::TRANSITION_IN);
            setIdleTimerDisabled(false);
            break;
		case PlayfieldController::PLAYING:
            pushFocusState(CMInputs::FOCUS_STATE_PF_PLAYFIELD);
            setIdleTimerDisabled(true);
            break;
    }
}

void PlayfieldController::setIdleTimerDisabled(bool disabled)
{
    if (disabled != mIsIdleTimerDisabled)
    {
        mIsIdleTimerDisabled = disabled;
        BridgingUtility::setIdleTimerDisabled(disabled);
        
        if (disabled && mTouchManager)
            mTouchManager->setTimeSinceLastTouch(0);
    }
}

void PlayfieldController::proceedToNextUnsolvedPuzzle(void)
{
    processCloudEvents();
    
	PuzzleController* puzzleController = getPuzzleController();
	if (puzzleController)
    {
		PuzzleOrganizer* puzzleOrganizer = puzzleController->getPuzzleOrganizer();
		if (puzzleOrganizer)
		{
			int nextUnsolvedID = puzzleController->getNextUnsolvedPuzzleID();
			if (nextUnsolvedID != -1)
			{
				resetCurrentPuzzle_Internal();
				puzzleController->loadPuzzleByID(nextUnsolvedID);
				puzzleController->displayPuzzleRibbon();
			}
		}
    }
}

void PlayfieldController::puzzleWasSelectedAtMenu(int puzzleID)
{
	if (getState() == PlayfieldController::MENU && mMenuController && mMenuController->getState() == MenuController::MENU_IN)
    {
        PuzzleController* puzzleController = getPuzzleController();
        if (puzzleController) {
            getPuzzleController()->loadPuzzleByID(puzzleID);
            mMenuController->setState(MenuController::TRANSITION_OUT);
            setState(PlayfieldController::PLAYING);
        }
    }
}

int PlayfieldController::getPauseCategory(void) const
{
	return CMGlobals::HUD;
}

void PlayfieldController::onPausePressed(void)
{
	showEscDialog();
}

const char* PlayfieldController::puzzleDuration2Str(float duration)
{
    if (duration < 120.0f)
        return "0-2min";
    else if (duration < 300.0f)
        return "2-5min";
    else if (duration < 600.0f)
        return "5-10min";
    else if (duration < 900.0f)
        return "10-15min";
    else if (duration < 1800.0f)
        return "15-30min";
    else
        return "30min+";
}

std::string PlayfieldController::puzzleIndex2LevelSpanStr(int levelIndex)
{
    levelIndex = 1 + (levelIndex - levelIndex % 2); // [0->1, 1->1], [2->3, 3->3], [4->5, 5->5], ...
    std::string str = CMUtils::strConcatVal("", levelIndex).append(CMUtils::strConcatVal("-", levelIndex+1));
    return str;
}

void PlayfieldController::levelWasUnlocked(int levelIndex)
{
    if (levelIndex >= 0 && levelIndex < LevelMenu::kNumLevels)
    {
        const char* params[] = { "LevelName", LevelMenu::kLevelNames[levelIndex] };
        BridgingUtility::logEvent(BridgingUtility::EV_NAME_LEVEL_UNLOCKED, CMUtils::makeStrMap(params));
        if (mMenuController)
            mMenuController->showLevelUnlockedDialog("LevelUnlocked", levelIndex);
    }
}

void PlayfieldController::levelWasCompleted(int levelIndex)
{
    if (levelIndex >= 0 && levelIndex < LevelMenu::kNumLevels)
    {
        const char* params[] = { "LevelName", LevelMenu::kLevelNames[levelIndex] };
        BridgingUtility::logEvent(BridgingUtility::EV_NAME_LEVEL_COMPLETED, CMUtils::makeStrMap(params));
        if (mMenuController)
            mMenuController->showLevelCompletedDialog(levelIndex);
    }
}

void PlayfieldController::userRespondedToRatingPrompt(bool value)
{
    if (value)
    {
        std::string numSolvedStr = CMUtils::strConcatVal("", GameProgressController::GPC()->getNumSolvedPuzzles());
        const char* params[] = { "NumPuzzlesSolved", numSolvedStr.c_str() };
        BridgingUtility::logEvent(BridgingUtility::EV_NAME_RATED_THE_GAME, CMUtils::makeStrMap(params));
    }
    
    SceneController::userRespondedToRatingPrompt(value);
}

void PlayfieldController::enableCloud(bool enable)
{
    if (enable && mFpsSampler)
        mIsCloudEnableEnqueued = true;
    else
    {
        mIsCloudEnableEnqueued = false;
        SceneController::enableCloud(enable);
        
        if (enable)
            CCLog("iCLOUD ENABLED = TRUE >>>>>>>>");
        else
            CCLog("iCLOUD ENABLED = FALSE >>>>>>>>");
    }
}

void PlayfieldController::cloudInitDidComplete(bool enabled)
{
    if (mUserEnabledCloud)
    {
        SceneController::cloudInitDidComplete(enabled);
    
        if (!enabled)
        {
#ifdef CHEEKY_MOBILE
            std::string configStrings[] = { "ALERT_TITLE_iCloudUnavailable", "ALERT_MSG_iCloudUnavailable", "OK" };
            std::vector<std::string> config = CMUtils::makeVector(configStrings);
            BridgingUtility::showAlertView(this, EV_TYPE_CLOUD_UNAVAILABLE_ALERT(), config);
#else
            std::string configStrings[] = { "ALERT_TITLE_iCloudUnavailable", "ALERT_MSG_iCloudUnavailable", "OK" };
            std::vector<std::string> config = CMUtils::makeVector(configStrings);
            BridgingUtility::showAlertView(this, EV_TYPE_CLOUD_UNAVAILABLE_ALERT(), config, true);
#endif
        }
    }
    
    setUserDefault(CMSettings::B_CLOUD_ENABLED, enabled);
    flushUserDefaults();
    
    if (mMenuController)
        mMenuController->refreshCloudText();
    
    BridgingUtility::setCrashContext(BridgingUtility::bool2Context(enabled), BridgingUtility::CRASH_CONTEXT_ICLOUD);
}

std::string PlayfieldController::getEventPuzzleName(const Puzzle& puzzle)
{
    int levelNum = puzzle.getLevelIndex() + 1;
    std::string str(CMUtils::strConcatVal(levelNum < 10 ? "0" : "", levelNum));
    str.append(". ");
    str.append(puzzle.getName());
    return str;
}

std::string PlayfieldController::getEventPuzzleDuration(const Puzzle& puzzle, float duration)
{
    int puzzleNum = puzzle.getLevelIndex() * GameProgressController::GPC()->getNumPuzzlesPerLevel() + puzzle.getPuzzleIndex() + 1;
    std::string str(CMUtils::strConcatVal(puzzleNum < 10 ? "0" : "", puzzleNum));
    str += std::string(". ") + puzzle.getName() + " [" + puzzleDuration2Str(duration) + "]";
    return str;
}

void PlayfieldController::abortCurrentPuzzle(void)
{
    if (getPuzzleDuration() >= 20.0f) // Don't log trivial attempts
    {
        PuzzleController* puzzleController = getPuzzleController();
        if (puzzleController)
        {
            Puzzle* puzzle = puzzleController->getPuzzle();
            if (puzzle && !puzzle->isSolved())
                logPuzzleAnalyticsEvent(false);
        }
    }
}

void PlayfieldController::logPuzzleAnalyticsEvent(bool didSolve)
{
    PuzzleController* puzzleController = getPuzzleController();
    if (puzzleController)
    {
        Puzzle* puzzle = puzzleController->getPuzzle();
        if (puzzle)
        {
            // puzzleName: prepend level index for easy sorting in excel
            std::string puzzleName = getEventPuzzleName(*puzzle);
            // durationStr: prepend puzzle index and duration so that we can plot individual puzzle duration trends in excel
            std::string durationStr = getEventPuzzleDuration(*puzzle, getPuzzleDuration());
            {
                const char* params[] = {
                    "PuzzleName", puzzleName.c_str(),
                    "Duration", durationStr.c_str()
                };
                
                std::string evName = std::string(BridgingUtility::EV_NAME_PUZZLE_ATTEMPTED) + "_[" + getGameModeSuffix() + "]";
                std::map<std::string, std::string> strMap = CMUtils::makeStrMap(params);
                BridgingUtility::logEvent(evName.c_str(), strMap);
                
                if (didSolve)
                {
                    evName = std::string(BridgingUtility::EV_NAME_PUZZLE_SOLVED) + "_[" + getGameModeSuffix() + "]";
                    BridgingUtility::logEvent(evName.c_str(), strMap);
                }
            }
        }
    }
}

void PlayfieldController::resetCurrentPuzzle(void)
{
    SceneController::resetCurrentPuzzle();
    
    PuzzleController* puzzleController = getPuzzleController();
    if (puzzleController)
    {
        puzzleController->cancelEnqueuedActions();
        processMilestoneDialogs();
        resetCurrentPuzzle_Internal();
    }
}

void PlayfieldController::resetCurrentPuzzle_Internal(void)
{
    PuzzleController* puzzleController = getPuzzleController();
    if (puzzleController)
    {
        mIgnoreNewPuzzleEvents = true;
        puzzleController->resetCurrentPuzzle();
        mIgnoreNewPuzzleEvents = false;
    }
}

void PlayfieldController::serializeCurrentPuzzle(void)
{
#ifdef CHEEKY_MOBILE
    if (getState() == PlayfieldController::PLAYING)
    {
        if (SceneController::getUserDefaultInt(CMSettings::I_INTERRUPTED_PUZZLE_ID, -1) == -1)
        {
            PuzzleController* puzzleController = getPuzzleController();
            if (puzzleController && puzzleController->serializeCurrentPuzzle())
            {
                Puzzle* puzzle = puzzleController->getPuzzle();
                if (puzzle)
                {
                    SceneController::setUserDefault(CMSettings::I_INTERRUPTED_PUZZLE_ID, puzzle->getID());
                    SceneController::flushUserDefaults();
                }
            }
        }
    }
#endif
}

void PlayfieldController::returnToPuzzleMenu(void)
{
	if (getState() == PlayfieldController::PLAYING && mMenuController && mMenuController->getState() == MenuController::MENU_OUT)
    {
		PuzzleController* puzzleController = getPuzzleController();
        if (puzzleController)
        {
            puzzleController->cancelEnqueuedActions();
            
            Puzzle* puzzle = puzzleController->getPuzzle();
            PuzzleOrganizer* puzzleOrganizer = puzzleController->getPuzzleOrganizer();
            
            if (puzzleOrganizer && puzzle)
                mMenuController->jumpLevelMenuToLevel(
                    puzzleOrganizer->levelIndexForPuzzleID(puzzle->getID()),
                    puzzleOrganizer->levelBasedPuzzleIndexForPuzzleID(puzzle->getID()));
        }
        setState(PlayfieldController::MENU);
    }
}

void PlayfieldController::returnToLevelMenu(void)
{
	returnToPuzzleMenu();

    if (mMenuController)
        mMenuController->returnToLevelMenu();
}

void PlayfieldController::processMilestoneDialogs(void)
{
    if (mHasProcessedSolvedPuzzle || mMenuController == NULL)
        return;
    
    PuzzleController* puzzleController = getPuzzleController();
    if (puzzleController && puzzleController->didPuzzleGetSolved())
    {
        Puzzle* puzzle = puzzleController->getPuzzle();
        if (puzzle)
        {
            GameProgressController* gpc = GameProgressController::GPC();
            int levelIndex = puzzle->getLevelIndex();
            bool wasLevelCompleted = puzzleController->didPuzzleGetSolved() && gpc->getNumSolvedPuzzlesForLevel(levelIndex) == gpc->getNumPuzzlesPerLevel();
            
            if (gpc->getNumSolvedPuzzles() == gpc->getNumPuzzles())
            {
#if CM_BETA
                TFManager::TFM()->showThanksView();
#endif
                mMenuController->showPuzzleWizardDialog();
            }
            else if (levelIndex >= 0 && levelIndex < gpc->getNumLevels() - 1)
            {
                if (wasLevelCompleted)
                    levelWasCompleted(levelIndex);
                else if (puzzleController->didLevelUnlock())
                    levelWasUnlocked(levelIndex + 1);
#if CM_BETA
                else
                    showBetaIQRatingDialog();
#endif
            }
        }
    }
    
    mHasProcessedSolvedPuzzle = true;
}

void PlayfieldController::invalidateStateCaches(void)
{
	SceneController::invalidateStateCaches();

	purgeUniformLocations();
	generateMipMaps();
	setupPrograms(true);
	Localizer::invalidateStateCaches();
}

void PlayfieldController::resolutionDidChange(int width, int height)
{
    SceneController::resolutionDidChange(width, height);
    
    if (getPuzzleController())
        getPuzzleController()->resolutionDidChange();
    
    if (mTouchManager)
        mTouchManager->setTouchBounds(CCRectMake(0, 0,  getViewportWidth(), getViewportHeight()));
}

void PlayfieldController::onEvent(int evType, void* evData)
{
	SceneController::onEvent(evType, evData);

    if (evType == PuzzleController::EV_TYPE_PUZZLE_DID_BEGIN())
    {
        mHasProcessedSolvedPuzzle = mHasReportedBuggedPuzzle = false;
        //resetPauseButton();
        
        if (!mIgnoreNewPuzzleEvents)
        {
            setPuzzleDuration(0);
            
            PuzzleController* puzzleController = getPuzzleController();
            if (puzzleController)
            {
                Puzzle* puzzle = puzzleController->getPuzzle();
                if (puzzle)
                    BridgingUtility::setCrashContext(puzzle->getName(), BridgingUtility::CRASH_CONTEXT_PUZZLE_NAME);
            }
        }
    }
	else if (evType == PuzzleController::EV_TYPE_PUZZLE_SOLVED_ANIMATION_COMPLETED())
	{
        if (!mHasProcessedSolvedPuzzle)
        {
            if (mMenuController)
            {
                PuzzleController* puzzleController = getPuzzleController();
                if (puzzleController)
                {
                    Puzzle* puzzle = puzzleController->getPuzzle();
                    if (puzzle)
                    {
                        GameProgressController* gpc = GameProgressController::GPC();
                        int levelIndex = puzzle->getLevelIndex();
                        bool wasLevelCompleted = puzzleController->didPuzzleGetSolved() && gpc->getNumSolvedPuzzlesForLevel(levelIndex) == gpc->getNumPuzzlesPerLevel();
                        
                        if (getState() == PlayfieldController::PLAYING)
                        {
                            int nextUnsolvedID = puzzleController->getNextUnsolvedPuzzleID();
                            if (wasLevelCompleted || nextUnsolvedID == -1)
                                mMenuController->showMenuButton("MainMenu");
                            else
                                mMenuController->showMenuButton("NextUnsolvedPuzzle");
                        }

                        processMilestoneDialogs();
                    }
                }
            }
            
            mHasProcessedSolvedPuzzle = true;
        }
	}
    else if (evType == Puzzle::EV_TYPE_UNSOLVED_PUZZLE_WILL_SOLVE())
    {
        logPuzzleAnalyticsEvent(true);
    }
    else if (evType == MenuDialogManager::EV_TYPE_DID_CLOSE_LEVEL_COMPLETED_DIALOG())
    {
        bool wizardDidUnlock = false;
        PuzzleController* puzzleController = getPuzzleController();
        if (puzzleController && puzzleController->didLevelUnlock())
        {
            GameProgressController* gpc = GameProgressController::GPC();
            if (gpc->getNumSolvedPuzzlesForLevel(gpc->getNumLevels() - 1) == 0 && gpc->getNumSolvedPuzzles() == gpc->getNumPuzzles() - gpc->getNumPuzzlesPerLevel())
            {
                wizardDidUnlock = true;
                levelWasUnlocked(gpc->getNumLevels() - 1);
            }
        }
        
#if CM_BETA
        if (!wizardDidUnlock)
            showBetaIQRatingDialog();
#endif
    }
    else if (evType == FpsSampler::EV_TYPE_FPS_SAMPLER_COMPLETED())
    {
        if (mFpsSampler)
        {
#ifdef CHEEKY_MOBILE
    #if !CM_LOW_POWER
            double recFps = mFpsSampler->getRecommendedFps();
            if (recFps > 15)
            {
        #ifdef __ANDROID__
                if (recFps < 59)
                {
                    int dirtyRenderFps = recFps < 29.0 ? 20 : 30;
                    CMJniUtils::setDirtyRenderMode(dirtyRenderFps);
                }
        #else
                if (!GameController::GC()->isHighPerformanceMachine())
                    CCDirector::sharedDirector()->setAnimationInterval(1.0 / recFps);
        #endif

                CCLog("^^^^^^^^^^^^^^^^^^^^^^");
                CCLog("Raw FPS: %f", (float)mFpsSampler->getRawFps());
                CCLog("Recommended FPS: %f", (float)recFps);
                CCLog("^^^^^^^^^^^^^^^^^^^^^^");
            }
    #endif
#endif
            mHasSampledFps = true;
            finishFpsSample();
        }
    }
    else if (evType == CloudWrapper::getEvTypeLoggedIn())
    {
        setCloudEvent(kCloudEvLoggedIn, true);
    }
    else if (evType == CloudWrapper::getEvTypeLoggedOut())
    {
        setCloudEvent(kCloudEvLoggedOut, true);
    }
    else if (evType == CloudWrapper::getEvTypeDataChanged())
    {
        setCloudEvent(kCloudEvDataChanged, true);
    }
    else if (evType == CloudWrapper::getEvTypeSettingsChanged())
    {
        setCloudEvent(kCloudEvSettingsChanged, true);
    }
    else if (evType == CloudWrapper::getEvTypeUnavailable())
    {
        setCloudEvent(kCloudEvUnavailable, true);
    }
    else if (evType == EV_TYPE_CLOUD_UNAVAILABLE_ALERT())
    {
#ifdef CHEEKY_DESKTOP
        if (evData)
        {
            int buttonIndex = BridgingUtility::alertButtonReturnToZeroBasedIndex(*(int*)evData);
            if (buttonIndex == 1) // 0: Cancel, 1: OK
                BridgingUtility::openFileURL("/System/Library/PreferencePanes/iCloudPref.prefPane");
        }
#endif
    }
#ifdef __ANDROID__
    else if (evType == LicenseManager::EV_TYPE_LICENSE_DENIED())
    {
    	PuzzleController* puzzleController = getPuzzleController();
    	if (puzzleController)
    		puzzleController->enableLicenseLock(true);
    	if (mHasSetupCompleted)
    		GameProgressController::GPC()->forceSave();
    }
    else if (evType == LicenseManager::EV_TYPE_LICENSE_APPROVED())
    {
    	PuzzleController* puzzleController = getPuzzleController();
    	if (puzzleController)
    		puzzleController->enableLicenseLock(false);
    }
    else if (evType == LicenseManager::EV_TYPE_LICENSE_TRIAL_EXPIRED())
    {
    	PuzzleController* puzzleController = getPuzzleController();
    	if (puzzleController)
    		puzzleController->enableLicenseLock(true);
    	if (mHasSetupCompleted)
    		GameProgressController::GPC()->forceSave();
    }
    else if (evType == LicenseManager::EV_TYPE_LICENSE_TRIAL_EXTENDED())
	{
		PuzzleController* puzzleController = getPuzzleController();
		if (puzzleController)
			puzzleController->enableLicenseLock(false);
	}
    else if (evType == PuzzleController::EV_TYPE_LICENSE_LOCK_DID_TRIGGER())
    {
    	LicenseManager::LM()->showLicenseDialog();
    }
#endif
}

#if defined(TEST_REWARDS) && DEBUG
//float debugCountdown = 10;
//int testLevelIndex = 0;
#endif
void PlayfieldController::advanceTime(float dt)
{
#if CM_BETA
    // This should be the first call made in PlayfieldController::advanceTime()
    TFManager::TFM()->update(dt);
#endif
    
    if (mIsCloudEnableEnqueued && mFpsSampler == NULL)
        enableCloud(true);
    
    if (getState() == MENU || mUserEnabledCloud)
        processCloudEvents();
    
	SceneController::advanceTime(dt);

#ifdef __ANDROID__
	LicenseManager::LM()->update(dt, getState() == PLAYING ? dt : 0);
#endif

	if (mPuzzleController)
		mPuzzleController->advanceTime(dt);

	if (mMenuController)
    {
		mMenuController->advanceTime(dt);
        
        if (mTouchManager)
            mTouchManager->setTimeSinceLastTouch(mTouchManager->getTimeSinceLastTouch() + dt);
        
        // Update puzzle duration for analytics
        if (getState() == PLAYING && mMenuController->isEscDialogShowing() == false)
        {
            setPuzzleDuration(getPuzzleDuration() + dt);
            
            if (mTouchManager)
                setIdleTimerDisabled(mTouchManager->getTimeSinceLastTouch() < 90.0f);
        }
        else
        {
            if (mTouchManager)
                setIdleTimerDisabled(mTouchManager->getTimeSinceLastTouch() < 15.0f);
        }
        
        //if (mTouchManager)
        //    CCLog("mTouchManager->getTimeSinceLastTouch():%f", mTouchManager->getTimeSinceLastTouch());
    }

	if (mView)
		mView->advanceTime(dt);

#if defined(TEST_REWARDS) && DEBUG
//    debugCountdown -= dt;
//    if (debugCountdown <= 0)
//    {
//        debugCountdown = 10000;
//        if (testLevelIndex < 12) mMenuController->showLevelUnlockedDialog("LevelUnlocked", ++testLevelIndex);
//        //mMenuController->showLevelCompletedDialog(testLevelIndex++);
//        //if (testLevelIndex == 12) testLevelIndex = 0;
//        //mMenuController->showPuzzleWizardDialog();
//    }

//    debugCountdown -= dt;
//    if (debugCountdown <= 0) {
//        debugCountdown = 15;
//
//        CCLog("Puzzle: %d", PWDebug::puzzleCount);
//        CCLog("PuzzleBoard: %d", PWDebug::puzzleBoardCount);
//        CCLog("TilePiece: %d", PWDebug::tilePieceCount);
//        CCLog("Tile: %d", PWDebug::tileCount);
//        CCLog("TileDecoration: %d", PWDebug::tileDecorationCount);
//        CCLog("HumanPlayerPiece: %d", PWDebug::humanPlayerPieceCount);
//        CCLog("MirrorPlayerPiece: %d", PWDebug::mirrorPlayerPieceCount);
//        CCLog("Player: %d", PWDebug::playerCount);
//        CCLog("Shield: %d", PWDebug::shieldCount);
//        CCLog("End: ^^^^^^^^^^^^^^^^^^^^^^^^^^^");
//    }
    
//    if (mMiscTimer > 0)
//    {
//        mMiscTimer = MAX(0, mMiscTimer - dt);
//        if (mMiscTimer == 0)
//            //BridgingUtility::TF_showWelcomeView();
//            //BridgingUtility::TF_showFeedbackView(1);
//            //BridgingUtility::TF_showThanksView();
//    }
#endif
    
    ControlsManager::CM()->update();
}

void PlayfieldController::update(int controllerState)
{
#ifndef __ANDROID__
    if (ControlsManager::CM()->didKeyDepress(kVK_ANSI_Comma))
        playPrevJukeboxSong();
    else if (ControlsManager::CM()->didKeyDepress(kVK_ANSI_Period))  // Substitute for quick debugging: mJuggler->printContents();
        playNextJukeboxSong();
#endif
}
