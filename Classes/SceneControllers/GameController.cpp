
#include "GameController.h"
#include "AppMacros.h"
#include "PlayfieldController.h"
#include <Prop/UIControls/ProgressBar.h>
#include <Managers/InputManager.h>
#include <Utils/FloatTweener.h>
#include <Utils/ByteTweener.h>
#include <Utils/Utils.h>
#include <Utils/Localizer.h>
#include <Utils/LangFX.h>
#include <Utils/BridgingUtility.h>
USING_NS_CC;

const char* GameController::CM_NOTICE_GC_CONTENT_LOAD_BEGAN = "CM_NOTICE_GC_CONTENT_LOAD_BEGAN";
const char* GameController::CM_NOTICE_GC_CONTENT_LOAD_PROGRESSED = "CM_NOTICE_GC_CONTENT_LOAD_PROGRESSED";
const char* GameController::CM_NOTICE_GC_CONTENT_LOAD_COMPLETED = "CM_NOTICE_GC_CONTENT_LOAD_COMPLETED";

static Localizer::LocaleType s_launchLocale = Localizer::INVALID_LOCALE;
static GameController *g_sharedGameController = NULL;

static const int kSplashAnimatorLogoTag = 0;
static const int kSplashAnimatorTextTag = 1;
static const int kNumSplashAnimators = 2;
static const float kSplashTextPosYDivisor = 40.0f;

#pragma warning( disable : 4351 ) // new behavior: elements of array 'GameController::mSplashAnimatorsBacking' will be default initialized (changed in Visual C++ 2005)
GameController* GameController::GC(void)
{
	if (!g_sharedGameController)
	{
		g_sharedGameController = new GameController();
		g_sharedGameController->init();
	}
	return g_sharedGameController;
}

GameController::GameController(void)
:
mCurrentScene(NULL),
mUniqueKey(1),
mFps(60),
mContentLoadDuration(0),
mBgQuad(NULL),
mProgressBar(NULL),
mSplashNode(NULL),
mSplashMascot(NULL),
mSplashTweener(NULL),
mSplashAnimators(NULL),
mSplashAnimatorsBacking(),
mSplashScene(NULL),
mAppShouldExit(false),
mDidAppTerminate(false),
mSplashPaused(false)
{
    CCAssert(g_sharedGameController == NULL, "Attempted to allocate a second instance of a singleton.");
    mSplashAnimators = mSplashAnimatorsBacking;
    CCDirector::sharedDirector()->getScheduler()->scheduleUpdateForTarget(this, -1, false);
}

// Under what reasonable circumstance would this ever be called?
GameController::~GameController(void)
{
	CCDirector::sharedDirector()->getScheduler()->unscheduleUpdateForTarget(this);
    
    if (mSplashAnimators)
    {
        for (int i = 0; i < kNumSplashAnimators; ++i)
        {
            CC_SAFE_RELEASE_NULL(mSplashAnimators[i]);
        }
        mSplashAnimators = NULL;
    }
    
	CC_SAFE_RELEASE_NULL(mCurrentScene);
    CC_SAFE_RELEASE_NULL(mBgQuad);
    CC_SAFE_RELEASE_NULL(mProgressBar);
    CC_SAFE_RELEASE_NULL(mSplashNode);
    CC_SAFE_RELEASE_NULL(mSplashTweener);
    CC_SAFE_RELEASE_NULL(mSplashScene);
    mSplashMascot = NULL;
    g_sharedGameController = NULL; // Leave references valid
}

void GameController::init(void)
{
//    GLint maxTexUnits = -1;
//    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);
//    
//    if (maxTexUnits != -1 && maxTexUnits < 8)
//    {
//#ifdef CHEEKY_MOBILE
//        CCMessageBox("Your device is not supported. Puzzle Wizard execution will now pause...", "Device Not Supported");
//#else
//        CCMessageBox("Your system is not supported. Puzzle Wizard execution will now pause...", "System Not Supported");
//#endif
//        return;
//    }
    
    InputManager::IM()->enable(false);

    mCurrentScene = new PlayfieldController();
    mCurrentScene->beginContentLoad();

    // Colored Quad
	CCGLProgram* glProgram = new CCGLProgram();
	glProgram->initWithVertexShaderFilename("shaders/ColoredQuad.vsh", "shaders/ColoredQuad.fsh");
	CCShaderCache::sharedShaderCache()->addProgram(glProgram, "ColoredQuad");
    
	glProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
	glProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
	glProgram->link();
	glProgram->updateUniforms();
    
    mSplashScene = CCScene::create();
    mSplashScene->retain();
    
    mBgQuad = createBgQuad();
    mBgQuad->retain();
    mSplashScene->addChild(mBgQuad);
    
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    float maximizingScale = getMaximizingScale();
    
    mSplashNode = createSplashNode();
    mSplashNode->retain();
    mSplashNode->setScale(maximizingScale);
    mSplashNode->setPosition(ccp(winSize.width / 2, winSize.height / 2));
    CMUtils::setOpacity(mSplashNode, 0);
    mSplashScene->addChild(mSplashNode);
    
    // Position splash contents for animation and animate
    const float kLogoScaleFrom = 0.95f, kLogoScaleTo = 1.0f, kDuration = 2.0f, kDelay = 0.1f;
    {
        // Logo
        {
            CCNode* logoNode = (CCNode*)mSplashNode->getChildByTag(GameController::GC()->getSplashTagForKey("Logo"));
            logoNode->setScale(kLogoScaleFrom);
            
            FloatTweener* tweener = new FloatTweener(0, this, CMTransitions::EASE_OUT);
            tweener->reset(kLogoScaleFrom, kLogoScaleTo, kDuration, kDelay);
            tweener->setTag(kSplashAnimatorLogoTag);
            mSplashAnimators[tweener->getTag()] = tweener;
        }
        
        // Text
        {
            CCNode* textNode = (CCNode*)mSplashNode->getChildByTag(GameController::GC()->getSplashTagForKey("Text"));
            float splashTextOffsetY = -mCurrentScene->getViewHeight() / kSplashTextPosYDivisor;
            float fromY = textNode->getPositionY() + splashTextOffsetY, toY = textNode->getPositionY();
            textNode->setPositionY(fromY);
            
            FloatTweener* tweener = new FloatTweener(0, this, CMTransitions::EASE_OUT);
            tweener->reset(fromY, toY, kDuration, kDelay);
            tweener->setTag(kSplashAnimatorTextTag);
            mSplashAnimators[tweener->getTag()] = tweener;
        }
    }
    
    mSplashMascot = static_cast<CCSprite*>(mSplashNode->getChildByTag(getSplashTagForKey("Mascot")));
    mSplashMascot->setOpacity(255);
    
    mSplashTweener = new ByteTweener(0, this);
#ifdef __ANDROID__
    mSplashTweener->reset(0, 255, 0.4f * kDuration, kDelay); // 0, 255, 0.25f, 0.0f);
#elif defined(CHEEKY_MOBILE)
    mSplashTweener->reset(0, 255, 0.1f * kDuration, kDelay); // 0, 255, 0.1f, 0.0f);
#else
    //CMUtils::setOpacity(mSplashNode, 254);
    mSplashTweener->reset(0, 255,  0.4f * kDuration, kDelay); // 254, 255, 0.0f, 0.0f);
#endif
    mSplashPaused = true;
                          
    mProgressBar = createProgressBar();
    mProgressBar->retain();
    mProgressBar->setVisible(false);
    mSplashScene->addChild(mProgressBar);
    
    CCDirector::sharedDirector()->runWithScene(mSplashScene);
}

//GameController::GameController(const GameController& other) { CCAssert(false, "GameController is a singleton."; }
//GameController& GameController::operator=(const GameController& rhs) { CCAssert(false, "GameController is a singleton."; }

void GameController::update(float dt)
{
    dt = MIN(dt, 2.0f * CCDirector::sharedDirector()->getAnimationInterval()); // Filter lag spikes
    
    if (mCurrentScene)
    {
        if (mSplashPaused)
        {
            CCAssert(mSplashTweener, "Invalid Splash scene state: SplashTweener (null).");
            mSplashTweener->advanceTime(dt);
            
            if (mSplashAnimators)
            {
                for (int i = 0; i < kNumSplashAnimators; ++i)
                {
                    if (mSplashAnimators[i])
                        mSplashAnimators[i]->advanceTime(dt);
                }
            }
        }
        else
        {
            if (mSplashAnimators)
            {
                for (int i = 0; i < kNumSplashAnimators; ++i)
                {
                    if (mSplashAnimators[i])
                        mSplashAnimators[i]->advanceTime(dt);
                }
            }
            else if (!mCurrentScene->isContentLoaded())
            {
                mCurrentScene->loadNextContent();
                mContentLoadDuration += dt;
                
                if (mProgressBar)
                    mProgressBar->setProgress(getContentLoadProgress());
                
                if (mCurrentScene->isContentLoaded())
                {
                    mCurrentScene->willGainSceneFocus();
                    mCurrentScene->presentScene();
                    
                    CC_SAFE_RELEASE_NULL(mBgQuad);
                    CC_SAFE_RELEASE_NULL(mProgressBar);
                    CC_SAFE_RELEASE_NULL(mSplashNode);
                    CC_SAFE_RELEASE_NULL(mSplashTweener);
                    CC_SAFE_RELEASE_NULL(mSplashScene);
                    mSplashMascot = NULL;
                    // Leave splash textures in memory for use in rest of game
                    
                    CCDirector::sharedDirector()->setNextDeltaTimeZero(true); // Shed loading lag
                    BridgingUtility::postNotification(CM_NOTICE_GC_CONTENT_LOAD_COMPLETED);
                }
                else
                    BridgingUtility::postNotification(CM_NOTICE_GC_CONTENT_LOAD_PROGRESSED);
            }
            else
            {
                mCurrentScene->advanceTime(dt);
            }
        }
    }
}

CCSize GameController::getAdjustedWinSize(void)
{
	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	//winSize.width = 200; //*= 0.8f;
	//winSize.height = 400; //*= 0.2f;
	return winSize;
}

CCSize GameController::getProgressBarDimensions(void)
{
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    return CCSizeMake(0.6f * winSize.width, 4);
}

float GameController::getMaximizingScale(void)
{
	CCSize winSize = getAdjustedWinSize();
	return MIN(winSize.width / SceneController::kDefaultSceneWidth, winSize.height / SceneController::kDefaultSceneHeight);
}

CCSprite* GameController::createBgQuad(void)
{
    CCSize quadSize = getAdjustedWinSize();
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    CCSprite* bgQuad = CMUtils::createColoredQuad(CCSizeMake(quadSize.width, quadSize.height));
    bgQuad->setPosition(ccp(winSize.width / 2, winSize.height / 2));
    bgQuad->setColor(CMUtils::uint2color3B(0));
    return bgQuad;
}

ProgressBar* GameController::createProgressBar(void)
{
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    CCSize barSize = getProgressBarDimensions();
#ifdef CHEEKY_MOBILE
    ProgressBar* progressBar = ProgressBar::create(-1, barSize.width, barSize.height); // SceneController::kDefaultSceneWidth / 2, 4);
    progressBar->setPosition(ccp(winSize.width / 2, progressBar->boundingBox().size.height / 2 + winSize.height / 20));
    progressBar->setProgressColor(0x8214d2);
#else
    ProgressBar* progressBar = ProgressBar::create(-1, barSize.width, barSize.height);
    progressBar->setPosition(ccp(winSize.width / 2, progressBar->boundingBox().size.height / 2 + winSize.height / 20));
    progressBar->setProgressColor(0xa041e6);
#endif
    return progressBar;
}

void GameController::destroySplashNodeResources(void)
{
    Localizer::LocaleType locale = s_launchLocale;
    if (locale == Localizer::INVALID_LOCALE)
        return;
    
    // Game Logo
    {
        CCTextureCache::sharedTextureCache()->removeTextureForKey("lang/pw-logo-bg");
    }
    
    // Puzzle Text
    {
        const char* puzzleTexSuffix = LangFX::locale2PuzzleTexSuffix(locale);
        if (puzzleTexSuffix)
        {
            std::string puzzleTexKey = std::string("lang/pw-logo-puzzle-").append(puzzleTexSuffix);
            CCTextureCache::sharedTextureCache()->removeTextureForKey(puzzleTexKey.c_str());
        }
    }
    
    // Wizard Text
    {
        const char* wizardTexSuffix = LangFX::locale2WizardTexSuffix(locale);
        if (wizardTexSuffix)
        {
            std::string wizardTexKey = std::string("lang/pw-logo-wizard-").append(wizardTexSuffix);
            CCTextureCache::sharedTextureCache()->removeTextureForKey(wizardTexKey.c_str());
        }
    }
    
    // IQ Text
    {
        const char* iqStr = LangFX::locale2IQString(locale, false);
        if (iqStr)
        {
            std::string iqTexKey = std::string("lang/pw-logo-").append(iqStr);
            CCTextureCache::sharedTextureCache()->removeTextureForKey(iqTexKey.c_str());
        }
    }
    
    // Company Logo
    {
        CCTextureCache::sharedTextureCache()->removeTextureForKey("splash_mascot");
    }
}

CCNode* GameController::createSplashNode(void)
{
    CCNode* splashNode = new CCNode();
    splashNode->autorelease();
    
    ccTexParams params = {GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE};
    
    // Game Logo
    Localizer::LocaleType locale = Localizer::getLaunchLocale();
    if (s_launchLocale == Localizer::INVALID_LOCALE)
        s_launchLocale = locale;
    const char* logoKey = "lang/pw-logo-bg.png";
    CCTexture2D* logoTex = CCTextureCache::sharedTextureCache()->textureForKey(logoKey);
    if (logoTex == NULL)
    {
        logoTex = CCTextureCache::sharedTextureCache()->addImage(logoKey);
        logoTex->setTexParameters(&params);
        logoTex->generateMipmap();
    }
    
    CCSprite* logoSprite = CCSprite::createWithTexture(logoTex);
    logoSprite->setPosition(LangFX::getSplashLogoOffset(locale));
    logoSprite->setTag(getSplashTagForKey("Logo"));
    splashNode->addChild(logoSprite);
    
    // Container for all text sprites
    CCNode* textNode = new CCNode();
    textNode->autorelease();
    textNode->setTag(getSplashTagForKey("Text"));
    splashNode->addChild(textNode);
    
    // Puzzle Text
    {
        const char* puzzleTexSuffix = LangFX::locale2PuzzleTexSuffix(locale);
        if (puzzleTexSuffix)
        {
            std::string puzzleTexKey = std::string("lang/pw-logo-puzzle-").append(puzzleTexSuffix).append(".png");
            CCTexture2D* puzzleTex = CCTextureCache::sharedTextureCache()->textureForKey(puzzleTexKey.c_str());
            if (puzzleTex == NULL)
            {
                puzzleTex = CCTextureCache::sharedTextureCache()->addImage(puzzleTexKey.c_str());
                puzzleTex->setTexParameters(&params);
                puzzleTex->generateMipmap();
            }
            
            CCSprite* puzzleSprite = CCSprite::createWithTexture(puzzleTex);
            puzzleSprite->setPosition(LangFX::getSplashTextPuzzleOffset(locale));
            textNode->addChild(puzzleSprite);
        }
    }
    
    // Wizard Text
    {
        const char* wizardTexSuffix = LangFX::locale2WizardTexSuffix(locale);
        if (wizardTexSuffix)
        {
            std::string wizardTexKey = std::string("lang/pw-logo-wizard-").append(wizardTexSuffix).append(".png");
            CCTexture2D* wizardTex = CCTextureCache::sharedTextureCache()->textureForKey(wizardTexKey.c_str());
            if (wizardTex == NULL)
            {
                wizardTex = CCTextureCache::sharedTextureCache()->addImage(wizardTexKey.c_str());
                wizardTex->setTexParameters(&params);
                wizardTex->generateMipmap();
            }
            
            CCSprite* wizardSprite = CCSprite::createWithTexture(wizardTex);
            wizardSprite->setPosition(LangFX::getSplashTextWizardOffset(locale));
            textNode->addChild(wizardSprite);
        }
    }
    
    // IQ Text
    {
        const char* iqStr = LangFX::locale2IQString(locale, false);
        if (iqStr)
        {
            std::string iqTexKey = std::string("lang/pw-logo-").append(iqStr).append(".png");
            CCTexture2D* iqTex = CCTextureCache::sharedTextureCache()->textureForKey(iqTexKey.c_str());
            if (iqTex == NULL)
            {
                iqTex = CCTextureCache::sharedTextureCache()->addImage(iqTexKey.c_str());
                iqTex->setTexParameters(&params);
                iqTex->generateMipmap();
            }
            
            CCSprite* iqSprite = CCSprite::createWithTexture(iqTex);
            iqSprite->setPosition(LangFX::getSplashTextIqOffset(locale));
            //iqSprite->setVisible(false);
            textNode->addChild(iqSprite);
        }
    }
    
    // Company Logo
    {
        CCTexture2D* mascotTex = CCTextureCache::sharedTextureCache()->textureForKey("splash_mascot.png");
        if (mascotTex == NULL)
        {
            mascotTex = CCTextureCache::sharedTextureCache()->addImage("splash_mascot.png");
            mascotTex->setTexParameters(&params);
            mascotTex->generateMipmap();
        }
        
        CCSize winSize = getAdjustedWinSize();
        float maximizingScale = getMaximizingScale();
        
        CCSize mascotPosSize = CCSizeMake(
                                          winSize.width / maximizingScale,
                                          winSize.height / maximizingScale);
        
        CCSprite* mascotSprite = CCSprite::createWithTexture(mascotTex);
        mascotSprite->setTag(getSplashTagForKey("Mascot"));
        mascotSprite->setPosition(ccp(
                                      0.5f * mascotPosSize.width - 0.5f * mascotSprite->boundingBox().size.width,
                                      0.5f * mascotSprite->boundingBox().size.height - 0.5f * mascotPosSize.height));
        splashNode->addChild(mascotSprite);
        
#ifdef CHEEKY_DESKTOP
        mascotSprite->setVisible(false);
#endif
    }
    
    return splashNode;
}

static std::map<std::string, int> s_splashTags;
int GameController::getSplashTagForKey(const char* key)
{
    if (s_splashTags.empty())
    {
        s_splashTags["Logo"] = 0x1000;
        s_splashTags["Text"] = 0x1001;
        s_splashTags["Mascot"] = 0x1002;
    }
    
    std::map<std::string, int>::iterator it = s_splashTags.find(key);
    return it != s_splashTags.end() ? it->second : -1;
}

void GameController::onEvent(int evType, void* evData)
{
    if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
    {
        ByteTweener* tweener = (ByteTweener*)evData;
        if (tweener && tweener == mSplashTweener)
        {
            if (mSplashNode)
                CMUtils::setOpacity(mSplashNode, tweener->getTweenedValue());
            if (mSplashMascot)
                mSplashMascot->setOpacity(255);
        }
    }
    else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
    {
        ByteTweener* tweener = (ByteTweener*)evData;
        if (tweener && tweener == mSplashTweener)
        {
            mSplashPaused = false;
            CCAssert(mProgressBar, "Invalid Splash scene state: ProgressBar (null).");
            BridgingUtility::postNotification(CM_NOTICE_GC_CONTENT_LOAD_BEGAN);
        }
    }
    else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
            switch (tweener->getTag())
            {
                case kSplashAnimatorLogoTag:
                {
                    if (mSplashNode)
                    {
                        CCNode* logoNode = (CCNode*)mSplashNode->getChildByTag(GameController::GC()->getSplashTagForKey("Logo"));
                        logoNode->setScale(tweener->getTweenedValue());
                    }
                }
                    break;
                case kSplashAnimatorTextTag:
                {
                    if (mSplashNode)
                    {
                        CCNode* textNode = (CCNode*)mSplashNode->getChildByTag(GameController::GC()->getSplashTagForKey("Text"));
                        textNode->setPositionY(tweener->getTweenedValue());
                    }
                }
                    break;
                default:
                    break;
            }
		}
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
        FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
            if (mSplashAnimators)
            {
                mSplashAnimators[tweener->getTag()] = NULL; // Tweener is released below.
                // Check if all done
                int i = 0;
                do
                {
                    if (mSplashAnimators[i])
                        break;
                    if (++i == kNumSplashAnimators)
                    {
                        mSplashAnimators = NULL;
                        if (mProgressBar)
                            mProgressBar->setVisible(true);
                    }
                } while (mSplashAnimators);
            }
            tweener->setListener(NULL);
			tweener->autorelease();
        }
	}
}

void GameController::resolutionDidChange(int width, int height)
{
    if (mCurrentScene)
        mCurrentScene->resolutionDidChange(width, height);
    
    if (mSplashNode && mBgQuad && mProgressBar && mSplashMascot)
    {
        CCSize winSize = CCDirector::sharedDirector()->getWinSize();
        mBgQuad->setTextureRect(CCRectMake(0, 0, winSize.width, winSize.height));
        mBgQuad->setPosition(ccp(mBgQuad->boundingBox().size.width / 2, mBgQuad->boundingBox().size.height / 2));
        
        float maximizingScale = getMaximizingScale();
        mSplashNode->setScale(maximizingScale);
        mSplashNode->setPosition(ccp(winSize.width / 2, winSize.height / 2));
        
        CCSize barSize = getProgressBarDimensions();
        mProgressBar->reconfigure(barSize.width, barSize.height);
        mProgressBar->setPosition(ccp(winSize.width / 2, mProgressBar->boundingBox().size.height / 2 + winSize.height / 20));
        
        CCSize mascotPosSize = CCSizeMake(
                                          winSize.width / maximizingScale,
                                          winSize.height / maximizingScale);
        mSplashMascot->setPosition(ccp(
                                       0.5f * mascotPosSize.width - 0.5f * mSplashMascot->boundingBox().size.width,
                                       0.5f * mSplashMascot->boundingBox().size.height - 0.5f * mascotPosSize.height));
    }
}

void GameController::registerForEvent(int evType, IEventListener* listener)
{
    mEventDispatcher.addEventListener(evType, listener);
        
}

void GameController::deregisterForEvent(int evType, IEventListener* listener)
{
    mEventDispatcher.removeEventListener(evType, listener);
}

void GameController::notifyEvent(int evType, void* evData)
{
    mEventDispatcher.dispatchEvent(evType, evData);
}

void GameController::applicationDidEnterBackground(void)
{
    if (mCurrentScene)
        mCurrentScene->applicationDidEnterBackground();
}

void GameController::applicationWillEnterForeground(void)
{
    if (mCurrentScene)
        mCurrentScene->applicationWillEnterForeground();
    CCDirector::sharedDirector()->setNextDeltaTimeZero(true); // Shed resumption lag
}

void GameController::applicationWillTerminate(void)
{
    if (mDidAppTerminate)
        return;
    
    CCLog("**************");
    CCLog("Terminating...");
    CCLog("**************");
    
    if (mCurrentScene)
        mCurrentScene->applicationWillTerminate();
    CCDirector::sharedDirector()->getScheduler()->unscheduleUpdateForTarget(this);
    CCDirector::sharedDirector()->stopAnimation();
//    CCEGLView* openglView = CCDirector::sharedDirector()->getOpenGLView();
//    if (openglView)
//        openglView->end();
//    CCDirector::sharedDirector()->end();
    
    mDidAppTerminate = true;
}

void GameController::splashViewDidHide(void)
{
    if (mCurrentScene)
        mCurrentScene->splashViewDidHide();
}

void GameController::invalidateStateCaches(void)
{
	if (mCurrentScene)
		mCurrentScene->invalidateStateCaches();
}

bool GameController::isHighPerformanceMachine(void) const
{
    return BridgingUtility::isHighPerformanceMachine();
}

float GameController::getContentLoadProgress(void)
{
    return mCurrentScene ? mCurrentScene->getContentLoadProgress() : 0;
}

void GameController::exitApp(void)
{
#ifdef CHEEKY_DESKTOP
	mAppShouldExit = true;
#elif defined(__ANDROID__)
	CCDirector::sharedDirector()->end();
#endif
}

