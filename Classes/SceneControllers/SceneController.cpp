
#include "SceneController.h"

#ifdef __APPLE__
#include "../../../../CocosDenshion/include/SimpleAudioEngine.h"
#elif defined (__ANDROID__)
#include "SimpleAudioEngine.h"
#else
#include "../CocosDenshion/include/SimpleAudioEngine.h"
#endif

#include "AppMacros.h"
#include <Prop/Prop.h>
#include <Prop/CMSprite.h>
#include <Prop/UIControls/MenuButton.h>
#include <Interfaces/IResDependent.h>
#include <Managers/TouchManager.h>
#include <Managers/InputManager.h>
#include <SceneControllers/GameController.h>
#include <Utils/Jukebox.h>
#include <Utils/CloudWrapper.h>
#include <Utils/Utils.h>
#include <Utils/FloatTweener.h>
#include <Utils/ByteTweener.h>
#include <Utils/BridgingUtility.h>
#include <algorithm>
USING_NS_CC;

typedef std::map<int, std::string> FontMap;

const float SceneController::kDefaultSceneWidth = 1024.0f;
const float SceneController::kDefaultSceneHeight = 768.0f;

static bool mIsUserDefaultsDirty = false;

SceneController::SceneController(void)
    :
mIsSfxMuted(false),
mIsRestoringPreviousSession(false),
mIsSfxEnabled(true),
mIsMusicEnabled(true),
mWasMusicPlaying(false),
mLocked(false),
mDestructLock(false),
mIsJukeboxWaiting(false),
mUserEnabledCloud(false),
mViewWidth(kDefaultSceneWidth),
mViewHeight(kDefaultSceneHeight),
mTimeDelta(0),
mNumResumeMusicAttempts(0),
mResumeMusicInterval(0),
mPuzzleDuration(0),
mSpriteLayerManager(NULL),
mTouchManager(NULL),
mJukebox(NULL),
mPauseProp(NULL),
mEscKeyProp(NULL)
{
	mProps = CCArray::createWithCapacity(100);
	mProps->retain();

	mAdvProps = CCArray::createWithCapacity(50);
	mAdvProps->retain();

	mPropsAddQueue = CCArray::createWithCapacity(10);
	mPropsAddQueue->retain();

	mPropsRemoveQueue = CCArray::createWithCapacity(10);
	mPropsRemoveQueue->retain();

	mJuggler = new Juggler();

	mScene = CCScene::create();
	CCAssert(mScene, "Failed to create SceneController::mScene");
	mScene->retain();

	mBaseSprite = Prop::createWithCategory(0, false);
	CCAssert(mBaseSprite, "Failed to create SceneController::mBaseSprite");
	mScene->addChild(mBaseSprite);

	reconfigureBaseSprite();
    
    // Jukebox
#if defined(__APPLE__) || defined(__ANDROID__)
    std::map<std::string, double> songs;
	songs["01-The_tale_of_room_620-Ehren_Starks"] = 5 * 60 + 24;
    songs["02-Sunset_in_Pensacola-Ehren_Starks"] = 4 * 60 + 48;
    songs["04-Slippolska-Erik_Ask_Upmark"] = 3 * 60 + 9;
    songs["09-Florellen-Erik_Ask_Upmark"] = 4 * 60 + 22;
    songs["08-Blekingarna-Erik_Ask_Upmark"] = 4 * 60 + 41;
//    songs["09-Hidden_Sky-Jami_Sieber"] = 7 * 60 + 6;
    #ifdef CHEEKY_DESKTOP
        songs["08-Virgin_Light-Cheryl_Ann_Fulton"] = 6 * 60 + 2;
    #endif

    //#if defined(__APPLE__)
        Jukebox::JukeboxType jbType = Jukebox::JB_EVENT_BASED;
    //#elif defined(__ANDROID__)
    //    Jukebox::JukeboxType jbType = Jukebox::JB_MANUAL_TIMER;
    //#endif
    
    mJukebox = new Jukebox(jbType, &songs, this);
    mJukebox->randomize();
    
    BridgingUtility::applyDefaultResignActiveBehavior();
    BridgingUtility::enableBackgroundAudioCompletionListening(true);
#endif
}

SceneController::~SceneController(void)
{
	mDestructLock = true;
	CC_SAFE_RELEASE(mPauseProp);
	CC_SAFE_RELEASE(mBaseSprite);
	CC_SAFE_RELEASE(mProps);
	CC_SAFE_RELEASE(mAdvProps);
	CC_SAFE_RELEASE(mPropsAddQueue);
	CC_SAFE_RELEASE(mPropsRemoveQueue);
	CC_SAFE_RELEASE(mScene);
	Prop::relinquishPropsScene(this);
    IAnimatable::relinquishAnimatablesScene(this);
	CMSprite::relinquishCMSpriteScene(this);
	CocosDenshion::SimpleAudioEngine::sharedEngine()->stopAllEffects();
	CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
    delete mJukebox; mJukebox = NULL;
}

void SceneController::preSetupController(void)
{
    Prop::setPropsScene(this);
    IAnimatable::setAnimatablesScene(this);
	CMSprite::setCMSpriteScene(this);
}

void SceneController::willGainSceneFocus(void)
{
	// In case another scene took it in the meantime
	Prop::setPropsScene(this);
    IAnimatable::setAnimatablesScene(this);
	CMSprite::setCMSpriteScene(this);
    attachEventListeners();
}

void SceneController::willLoseSceneFocus(void)
{
    detachEventListeners();
}

void SceneController::presentScene(void)
{
	CCDirector::sharedDirector()->replaceScene(mScene);
	//CCDirector::sharedDirector()->runWithScene(mScene);
}

void SceneController::applyGameSettings(void)
{
#ifdef CHEEKY_MOBILE
	setMusicVolume(CMSettings::kDefaultMusicVolume);
	setSfxVolume(CMSettings::kDefaultSfxVolume);
    enableSfx(getUserDefaultBool(CMSettings::B_SFX, true));
	enableMusic(getUserDefaultBool(CMSettings::B_MUSIC, true));
#else
    setMusicVolume(getUserDefaultInt(CMSettings::I_MUSIC, CMSettings::kDefaultMusicVolume));
    setSfxVolume(getUserDefaultInt(CMSettings::I_SFX, CMSettings::kDefaultSfxVolume));
#endif
}

void SceneController::applicationDidEnterBackground(void)
{
#if defined(CHEEKY_DESKTOP) || defined(__ANDROID__)
    CocosDenshion::SimpleAudioEngine::sharedEngine()->stopAllEffects();
    
    if (isMusicEnabled())
    {
        if (mJukebox)
            mJukebox->pause();
        CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
        CCLog("Music paused.");
    }
#endif
}

void SceneController::applicationWillEnterForeground(void)
{
#if defined(CHEEKY_DESKTOP)
	if (isMusicEnabled())
	{
		if (CocosDenshion::SimpleAudioEngine::sharedEngine()->willPlayBackgroundMusic())
			CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();

		if (CocosDenshion::SimpleAudioEngine::sharedEngine()->isBackgroundMusicPlaying())
		{
			if (mJukebox)
				mJukebox->resume();
			CCLog("Music resumed.");
		}
		else
		{
			if (mJukebox)
				mJukebox->pause();
			setResumeMusicAttempts(10);
		}
	}
#elif defined(CHEEKY_MOBILE)
	#if defined(__APPLE__)
		if (mJukebox)
			mJukebox->pause();
		setResumeMusicAttempts(10);
	#elif defined(__ANDROID__)
		if (isMusicEnabled())
		{
			if (CocosDenshion::SimpleAudioEngine::sharedEngine()->willPlayBackgroundMusic())
				CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();

			if (mJukebox)
			{
				mJukebox->resume();
				if (!CocosDenshion::SimpleAudioEngine::sharedEngine()->isBackgroundMusicPlaying())
				{
					playMusic(mJukebox->getCurrentSongName());
					mIsJukeboxWaiting = false;
					mJukebox->nextSong();
					CCLog("Music started.");
				}
				else
					CCLog("Music resumed.");
			}
		}
	#endif
#endif
    
    if (mTouchManager)
		mTouchManager->resetTouches();
}

void SceneController::applicationWillTerminate(void)
{
    if (mBaseSprite)
        mBaseSprite->removeFromParent();
}

void SceneController::splashViewDidHide(void)
{
    InputManager::IM()->enable(true);
}

void SceneController::invalidateStateCaches(void)
{
	// Do nothing
}

double SceneController::getFps(void) const
{
    return GameController::GC()->getFps();
}

float SceneController::getViewportWidth(void) const
{
	return CCDirector::sharedDirector()->getWinSize().width;
}

float SceneController::getViewportHeight(void) const
{
	return CCDirector::sharedDirector()->getWinSize().height;
}

float SceneController::getViewportScaledWidth(void) const
{
    return mBaseSprite && mBaseSprite->getScaleX() != 0 ? CCDirector::sharedDirector()->getWinSize().width / mBaseSprite->getScaleX() : 0.0f;
}

float SceneController::getViewportScaledHeight(void) const
{
    return mBaseSprite && mBaseSprite->getScaleY() != 0 ? CCDirector::sharedDirector()->getWinSize().height / mBaseSprite->getScaleY() : 0.0f;
}

vec2 SceneController::getViewScale(void) const
{
	return mBaseSprite ? cmv2(mBaseSprite->getScaleX(), mBaseSprite->getScaleY()) : cmv2(1, 1);
}

vec2 SceneController::getInvViewScale(void) const
{
	return mBaseSprite ? cmv2(1.0f / mBaseSprite->getScaleX(), 1.0f / mBaseSprite->getScaleY()) : cmv2(1, 1);
}

float SceneController::getViewAspectRatio(void) const
{
	return mViewWidth / mViewHeight;
}

float SceneController::getViewportAspectRatio(void) const
{
	return getViewportWidth() / MAX(1.0f, getViewportHeight());
}

float SceneController::getMaximizingContentScaleFactor(void) const
{
	return MAX(1.0f, getViewportAspectRatio() / getViewAspectRatio());
}

CCPoint SceneController::getContentOffset(void) const
{
	return mBaseSprite ? mBaseSprite->getPosition() : CCPointZero;
}

float SceneController::getBaseScale(void) const
{
    return mBaseSprite ? mBaseSprite->getScale() : 1.0f;
}

void SceneController::setColorScheme(Puzzles::ColorScheme value)
{
    if (value != getColorScheme())
    {
        Puzzles::setColorScheme(value);
        setUserDefault(CMSettings::B_COLOR_BLIND_MODE, value == Puzzles::CS_COLOR_BLIND);
        BridgingUtility::setCrashContext(
                                         BridgingUtility::bool2Context(value == Puzzles::CS_COLOR_BLIND),
                                         BridgingUtility::CRASH_CONTEXT_COLOR_BLIND);
    }
}

void SceneController::enableLowPowerMode(bool enable)
{
#ifdef CHEEKY_MOBILE
    CCDirector::sharedDirector()->setAnimationInterval(1.0 / (enable ? 20.0 : 30.0));
#endif
}

void SceneController::addFontName(const char* fontName, int fontSize)
{
	if (fontName && fontSize > 0)
		mFontNames[fontSize] = std::string(fontName);
}

void SceneController::removeFontName(int fontSize)
{
    if (mFontNames.count(fontSize) != 0)
        mFontNames.erase(fontSize);
}

const std::string& SceneController::getFontName(int fontSize)
{
	CCAssert(mFontNames.size() > 0, "SceneController::getFontName - no fonts available.");
	int closestKey = -1;
	// Try to find the closest size that is >= the requested size so that we can scale down.
	for (FontMap::iterator it = mFontNames.begin(); it != mFontNames.end(); ++it)
	{
		if (it->first >= fontSize)
		{
			if (closestKey == -1 || it->first - fontSize < closestKey - fontSize)
				closestKey = it->first;
		}
	}

	FontMap::iterator findIt = mFontNames.find(closestKey);

	if (findIt == mFontNames.end())
	{
		// Now just settle for the closest size.
		for (FontMap::iterator it = mFontNames.begin(); it != mFontNames.end(); ++it)
		{
			if (closestKey == -1 || abs(it->first - fontSize) < abs(closestKey - fontSize))
				closestKey = it->first;
		}

		findIt = mFontNames.find(closestKey);
	}

	CCAssert(findIt != mFontNames.end(), "SceneController::getFontName - bad state.");
	return findIt->second;
}

void SceneController::reconfigureBaseSprite(void)
{
	CCSize viewport = CCDirector::sharedDirector()->getWinSize();

	float baseScaleX = viewport.width / mViewWidth;
	float baseScaleY = viewport.height / mViewHeight;
    float baseScale = MIN(baseScaleX, baseScaleY);
	mBaseSprite->setScale(baseScale);
	mBaseSprite->setPosition(ccp(
		fabs(baseScale - baseScaleX) * mViewWidth / 2.0f,
		fabs(baseScale - baseScaleY) * mViewHeight / 2.0f));
}

void SceneController::registerResDependent(IResDependent* resDependent)
{
#ifdef CHEEKY_DESKTOP
    if (resDependent && !CMUtils::contains(mResDependents, resDependent))
        mResDependents.push_back(resDependent);
#endif
}

void SceneController::deregisterResDependent(IResDependent* resDependent)
{
#ifdef CHEEKY_DESKTOP
	if (resDependent)
        CMUtils::erase(mResDependents, resDependent);
#endif
}

void SceneController::resolutionDidChange(int width, int height)
{
#ifdef CHEEKY_DESKTOP
    reconfigureBaseSprite();
    updatePauseButton();
    updateEscKeyPrompt();
    
	// Clients know not to register/deregister via this call.
	for (std::vector<IResDependent*>::iterator it = mResDependents.begin(); it != mResDependents.end(); ++it)
		(*it)->resolutionDidChange();
#endif
}

void SceneController::registerLocalizable(ILocalizable* localizable)
{
    if (localizable && !CMUtils::contains(mLocalizables, localizable))
        mLocalizables.push_back(localizable);
}

void SceneController::deregisterLocalizable(ILocalizable* localizable)
{
    if (localizable)
        CMUtils::erase(mLocalizables, localizable);
}

void SceneController::localeDidChange(void)
{
#ifdef CHEEKY_DESKTOP
    updateEscKeyPrompt();
#endif
    
    const char* fontKey = getFontName(TextUtils::kBaseFontSize).c_str();
    const char* FXFontKey = getFontName(TextUtils::kBaseFXFontSize).c_str();
    for (std::vector<ILocalizable*>::iterator it = mLocalizables.begin(); it != mLocalizables.end(); ++it)
		(*it)->localeDidChange(fontKey, FXFontKey);
}

void SceneController::registerForTouches(CMTouches::TouchCategory category, IEventListener* listener)
{
#ifdef CHEEKY_MOBILE
	if (mTouchManager)
		mTouchManager->registerForTouches(category, listener);
#else
    if (mTouchManager && (category == CMTouches::TC_PAUSE || category == CMTouches::TC_MENU))
		mTouchManager->registerForTouches(category, listener);
#endif
}

void SceneController::deregisterForTouches(CMTouches::TouchCategory category, IEventListener* listener)
{
	if (mTouchManager)
		mTouchManager->deregisterForTouches(category, listener);
}

void SceneController::subscribeToInputUpdates(IInteractable* client, bool modal)
{
    InputManager::IM()->subscribe(client, modal);
}

void SceneController::unsubscribeToInputUpdates(IInteractable* client, bool modal)
{
    InputManager::IM()->unsubscribe(client, modal);
}

bool SceneController::hasInputFocus(uint focus)
{
    return InputManager::IM()->hasFocus(focus);
}

void SceneController::pushFocusState(uint focusState, bool modal)
{
    InputManager::IM()->pushFocusState(focusState, modal);
}

void SceneController::popFocusState(uint focusState, bool modal)
{
    InputManager::IM()->popFocusState(focusState, modal);
}

void SceneController::popToFocusState(uint focusState, bool modal)
{
    InputManager::IM()->popToFocusState(focusState, modal);
}

void SceneController::addToJuggler(IAnimatable* obj)
{
	if (mJuggler)
		mJuggler->addObject(obj);
}

void SceneController::removeFromJuggler(IAnimatable* obj)
{
	if (mJuggler)
		mJuggler->removeObject(obj);
}

void SceneController::removeTweensWithTarget(void* target)
{
	if (mJuggler)
		mJuggler->removeTweensWithTarget(target);
}

CCTexture2D* SceneController::textureByName(const char* name) const
{
	if (name)
	{
		std::string s = std::string(name).append(CM_TEX_EXT);
		CCTexture2D* texture = CCTextureCache::sharedTextureCache()->textureForKey(s.c_str());
        if (texture == NULL)
        {
            s = std::string(name).append(CM_TEX_ZIP_EXT);
            texture = CCTextureCache::sharedTextureCache()->textureForKey(s.c_str());
        }
        
        return texture;
	}
	else
		return NULL;
}

CCSpriteFrame* SceneController::spriteFrameByName(const char* name) const
{
	if (name)
	{
		std::string s = std::string(name).append(CM_TEX_EXT);
		return CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(s.c_str());
	}
	else
		return NULL;
}

CCArray* SceneController::spriteFramesStartingWith(const char* name) const
{
	if (name == NULL)
		return NULL;

	CCArray* frames = CCArray::create();
	for (int i = 0; i < 100; ++i)
	{
		std::ostringstream s;
		if (i < 10)
			s << name << "0" << i;
		else
			s << name << i;

		CCSpriteFrame* frame = this->spriteFrameByName(s.str().c_str());
		if (frame)
			frames->addObject(frame);
		else
			break;
	}

	return frames;
}

GLint SceneController::uniformLocationByName(const char* name)
{
	std::map<std::string, GLint>::iterator it = mUniformLocations.find(std::string(name));
	return it != mUniformLocations.end() ? (*it).second : -1;
}

void SceneController::setUniformLocation(const char* name, GLint uniformLoc)
{
	mUniformLocations[std::string(name)] = uniformLoc;
}

void SceneController::purgeUniformLocations(void)
{
	mUniformLocations.clear();
}

GLuint SceneController::textureUnitById(GLuint textureId)
{
	std::map<GLuint,GLuint>::iterator it = mTextureUnits.find(textureId);
	CCAssert(it != mTextureUnits.end(), "Texture unit cache was not fully initialized for use.");
	return it != mTextureUnits.end() ? (*it).second : 0;
}

void SceneController::setTextureUnit(GLuint textureId, GLuint textureUnit)
{
	mTextureUnits[textureId] = textureUnit;
}

void SceneController::setLocale(Localizer::LocaleType locale)
{
    if (locale != getLocale())
    {
        Localizer::LocaleType prevLocale = getLocale();
        Localizer::initLocalizationStrings(Localizer::EN, locale);
        Localizer::initLocalizationContent(locale);
        Localizer::setLocale(locale);
        Localizer::purgeLocale(prevLocale, locale);
        localeDidChange(); // This will repopulate the CCLabelBMFont caches as labels update their text
        setUserDefault(CMSettings::I_LOCALE, (int)locale);
        BridgingUtility::setCrashContext(Localizer::locale2String(locale), BridgingUtility::CRASH_CONTEXT_LOCALE);
    }
}

void SceneController::enableSfx(bool enable)
{
	if (enable != mIsSfxEnabled)
	{
		mIsSfxEnabled = enable;
		this->setUserDefault(CMSettings::B_SFX, mIsSfxEnabled);
	}
}

void SceneController::enableMusic(bool enable)
{
	if (enable != mIsMusicEnabled)
	{
        if (enable)
        {
            if (CocosDenshion::SimpleAudioEngine::sharedEngine()->willPlayBackgroundMusic())
            {
                // Make sure the internal state is valid for resume to be serviced.
                CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
                CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
                mWasMusicPlaying = CocosDenshion::SimpleAudioEngine::sharedEngine()->isBackgroundMusicPlaying();
                if (mWasMusicPlaying)
                    setResumeMusicAttempts(0);
                //CCLog("SceneController::enableMusic - status: %s", mWasMusicPlaying ? "enabled" : "disabled");
            }
            //else
            //    CCLog("SceneController::enableMusic - status: aborted");
        }
        else
        {
            CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
        }
        
        mIsMusicEnabled = enable;
		this->setUserDefault(CMSettings::B_MUSIC, mIsMusicEnabled);
	}
    
    if (mJukebox)
    {
        if (enable)
            mJukebox->resume();
        else
            mJukebox->pause();
    }
    
}

int SceneController::getSfxVolume(void) const
{
    return (int)(CocosDenshion::SimpleAudioEngine::sharedEngine()->getEffectsVolume() * CMSettings::kMaxVolume);
}

void SceneController::setSfxVolume(int value)
{
    int volume = MAX(0, MIN(value, CMSettings::kMaxVolume));
    if (getSfxVolume() != volume)
    {
        CocosDenshion::SimpleAudioEngine::sharedEngine()->setEffectsVolume(volume / (float)CMSettings::kMaxVolume);
        this->setUserDefault(CMSettings::I_SFX, volume);
        std::string volumeStr = CMUtils::strConcatVal("", volume);
        BridgingUtility::setCrashContext(volumeStr.c_str(), BridgingUtility::CRASH_CONTEXT_SFX);
#ifdef CHEEKY_DESKTOP
        enableSfx(volume != 0);
#endif
    }
}

int SceneController::getMusicVolume(void) const
{
    return (int)(CocosDenshion::SimpleAudioEngine::sharedEngine()->getBackgroundMusicVolume() * CMSettings::kMaxVolume);
}

void SceneController::setMusicVolume(int value)
{
    int volume = MAX(0, MIN(value, CMSettings::kMaxVolume));
    if (getMusicVolume() != volume)
    {
        CocosDenshion::SimpleAudioEngine::sharedEngine()->setBackgroundMusicVolume(volume / (float)CMSettings::kMaxVolume);
        setUserDefault(CMSettings::I_MUSIC, volume);
        std::string volumeStr = CMUtils::strConcatVal("", volume);
        BridgingUtility::setCrashContext(volumeStr.c_str(), BridgingUtility::CRASH_CONTEXT_MUSIC);
#ifdef CHEEKY_DESKTOP
        enableMusic(volume != 0);
#endif
    }
}

unsigned int SceneController::playSound(std::string name)
{
	if (isSfxEnabled() && !isSfxMuted() && !name.empty())
	{
		std::string s = std::string(name).append(CM_SND_EXT);
		return CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(s.c_str());
	}
    else
        return CM_MUTE;
}

void SceneController::stopSound(unsigned int soundId)
{
    CocosDenshion::SimpleAudioEngine::sharedEngine()->stopEffect(soundId);
}

void SceneController::playMusic(std::string name, bool loop)
{
	if (isMusicEnabled() && !name.empty())
	{
        if (CocosDenshion::SimpleAudioEngine::sharedEngine()->willPlayBackgroundMusic())
        {
            std::string s = std::string(name).append(CM_MUSIC_EXT);
            CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic(s.c_str(), loop);
            mWasMusicPlaying = CocosDenshion::SimpleAudioEngine::sharedEngine()->isBackgroundMusicPlaying();
            
            if (mWasMusicPlaying)
            {
                setResumeMusicAttempts(0);
                if (mJukebox)
                    mJukebox->resume();
            }
        }
		else
            mWasMusicPlaying = false;
        
        //CCLog("SceneController::playMusic - status: %s", mWasMusicPlaying ? "playing" : "not playing");
	}
}

std::string SceneController::getCurrentJukeboxSongName(void) const
{
    return mJukebox ? mJukebox->getCurrentSongName() : "";
}

std::string SceneController::getPreviousJukeboxSongName(void) const
{
    return mJukebox ? mJukebox->getPreviousSongName() : "";
}

std::string SceneController::playPrevJukeboxSong(bool loop)
{
    if (mJukebox && CocosDenshion::SimpleAudioEngine::sharedEngine()->willPlayBackgroundMusic())
    {
        CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic(true);
        mJukebox->prevSong();
        BridgingUtility::didBackGroundAudioComplete(); // Clear the flag
    }
    
    return getCurrentJukeboxSongName();
}

std::string SceneController::playNextJukeboxSong(bool loop)
{
    if (mJukebox && CocosDenshion::SimpleAudioEngine::sharedEngine()->willPlayBackgroundMusic())
    {
        CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic(true);
        mJukebox->nextSong();
        BridgingUtility::didBackGroundAudioComplete(); // Clear the flag
    }
    
    return getCurrentJukeboxSongName();
}

void SceneController::setResumeMusicAttempts(int value)
{
#ifdef __APPLE__
    mNumResumeMusicAttempts = value;
    mResumeMusicInterval = 0.5f;
#endif
}

void SceneController::resumeMusicFromSystemPause(float dt)
{
    if (mNumResumeMusicAttempts > 0)
    {
        mResumeMusicInterval -= dt;
        
        if (mResumeMusicInterval <= 0)
        {
            setResumeMusicAttempts(mNumResumeMusicAttempts-1);
            
            if (BridgingUtility::attemptAudioRestart())
            {
                if (isMusicEnabled())
                {
                    if (mWasMusicPlaying)
                    {
                        mIsMusicEnabled = false;
                        enableMusic(true);
                    }
                    else
                    {
                        if (mJukebox)
                            playMusic(getCurrentJukeboxSongName());
                    }
                }
            }
        }
    }
}

void SceneController::advanceTime(float dt)
{
	mTimeDelta = dt;

    resumeMusicFromSystemPause(dt);
    InputManager::IM()->update();
    
	mLocked = true; // *** LOCK ***

	CCObject* child;
	CCARRAY_FOREACH(mAdvProps, child)
	{
		Prop* advProp = static_cast<Prop*>(child);
		if(advProp)
			advProp->advanceTime(dt);
	}

	mJuggler->advanceTime(dt);

	mLocked = false; // *** UNLOCK ***

	this->removeQueuedProps();
	this->addQueuedProps();

    if (mJukebox)
    {
        if (mIsJukeboxWaiting && isMusicEnabled())
        {
            if (!CocosDenshion::SimpleAudioEngine::sharedEngine()->isBackgroundMusicPlaying())
            {
                playMusic(mJukebox->getCurrentSongName());
                mIsJukeboxWaiting = false;
                mJukebox->nextSong();
            }
        }
        
        mJukebox->advanceTime(dt);
    }
}

void SceneController::addProp(Prop* prop)
{
	if (mDestructLock || prop == NULL)
		return;
	if (mLocked)
		mPropsAddQueue->addObject(prop);
	else
	{
		mProps->addObject(prop);

		if (prop->isAdvanceable())
			mAdvProps->addObject(prop);
		mSpriteLayerManager->addChild(prop, prop->getCategory());
	}
}

void SceneController::removeProp(Prop* prop, bool tryCache)
{
	if (mDestructLock || prop == NULL)
		return;
	if (mLocked)
	{
		prop->setTryCache(tryCache);
		mPropsRemoveQueue->addObject(prop);
	}
	else
	{
		mSpriteLayerManager->removeChild(prop, prop->getCategory());
		if (prop->isAdvanceable())
			mAdvProps->removeObject(prop);
		mProps->removeObject(prop);

		if (tryCache)
			prop->tryCache();
	}
}

void SceneController::addQueuedProps()
{
	CCAssert(!mLocked, "SceneController::addQueuedProps attempted when mLocked == true");

	if (mPropsAddQueue->count() > 0)
	{
		CCObject* child;
		CCARRAY_FOREACH(mPropsAddQueue, child)
		{
			Prop* prop = static_cast<Prop*>(child);
			if(prop)
				this->addProp(prop);
		}

		mPropsAddQueue->removeAllObjects();
	}
}

void SceneController::removeQueuedProps()
{
	CCAssert(!mLocked, "SceneController::removeQueuedProps attempted when mLocked == true");

	if (mPropsRemoveQueue->count() > 0)
	{
		CCObject* child;
		CCARRAY_FOREACH(mPropsRemoveQueue, child)
		{
			Prop* prop = static_cast<Prop*>(child);
			if(prop)
				this->removeProp(prop, prop->getTryCache());
		}

		mPropsRemoveQueue->removeAllObjects();
	}
}

void SceneController::addPauseButton(const char* path)
{
	removePauseButton();

	if (mPauseProp == NULL)
	{
		mPauseProp = new Prop(getPauseCategory());
		addProp(mPauseProp);
	}

	MenuButton* pauseButton = MenuButton::create(spriteFrameByName(path), NULL, this);
	pauseButton->setTag(kTagPauseButton);
	pauseButton->enableTouch(true, CMTouches::TC_PAUSE);
	//pauseButton->setEnabledOpacity((GLubyte)(0.5f * 255));
	mPauseProp->addChild(pauseButton);
    updatePauseButton();
}

void SceneController::updatePauseButton(void)
{
    if (mBaseSprite && mPauseProp)
    {
        CCNode* pauseButton = mPauseProp->getChildByTag(kTagPauseButton);
        if (pauseButton)
        {
#ifdef CHEEKY_MOBILE
            vec2 pauseScale = IS_TABLET ? vec2(1.3f, 1.3f) : vec2(1.2f, 1.2f);
#else
            vec2 pauseScale = vec2(1.0f, 1.0f);
#endif
            pauseButton->setScaleX(pauseScale.x);
            pauseButton->setScaleY(pauseScale.y);
            
            CCRect pauseBounds = pauseButton->boundingBox();
            CCPoint nodePos = mBaseSprite->convertToNodeSpace(ccp(getViewportWidth(), getViewportHeight()));
            mPauseProp->setPosition(ccp(nodePos.x - pauseBounds.size.width / 2, nodePos.y - pauseBounds.size.height / 2));
        }
    }
}

void SceneController::resetPauseButton(void)
{
    if (mPauseProp)
    {
        MenuButton* pauseButton = static_cast<MenuButton*>(mPauseProp->getChildByTag(kTagPauseButton));
        if (pauseButton)
            pauseButton->reset();
    }
}

void SceneController::removePauseButton(void)
{
	if (mPauseProp)
		mPauseProp->removeAllChildren();
}

static const int kTagEscKeyPromptSprite = 0x0001;
static const int kTagEscKeyPromptTweenScale = 0x0002;
static const int kTagEscKeyPromptTweenAlpha = 0x0003;
void SceneController::addEscKeyPrompt(void)
{
    if (mEscKeyProp)
        return;
    
    mEscKeyProp = Prop::createWithCategory(getPauseCategory());
    mEscKeyProp->setAnchorPoint(ccp(0.0f, 0.0f));
    addProp(mEscKeyProp);
    
    CCSprite* escKeySprite = CCSprite::createWithSpriteFrame(spriteFrameByName("menu-esc"));
    escKeySprite->setTag(kTagEscKeyPromptSprite);
    escKeySprite->setAnchorPoint(ccp(0.0f, 0.0f));
    mEscKeyProp->addChild(escKeySprite);
    mEscKeyProp->setContentSize(escKeySprite->boundingBox().size);

    updateEscKeyPrompt();
}

void SceneController::removeEscKeyPrompt(void)
{
    mEscKeyProp->removeFromParent();
    mEscKeyProp = NULL;
}

void SceneController::showEscKeyPrompt(float duration, float delay)
{
    if (mEscKeyProp == NULL)
        addEscKeyPrompt();
    
    mEscKeyProp->setScale(0.0f);
    
    {
        FloatTweener* tweener = new FloatTweener(0, this, CMTransitions::LINEAR);
        tweener->reset(mEscKeyProp->getScaleX(), 1.0f, 1.0f, delay);
        tweener->setTag(kTagEscKeyPromptTweenScale);
        addToJuggler(tweener);
    }
    
    {
        ByteTweener* tweener = new ByteTweener(0, this, CMTransitions::LINEAR);
        tweener->reset(255, 0, 0.5f, duration + delay);
        tweener->setTag(kTagEscKeyPromptTweenAlpha);
        addToJuggler(tweener);
    }
}

void SceneController::updateEscKeyPrompt(void)
{
    if (mEscKeyProp == NULL)
        return;
    
    CCSprite* escKeySprite = (CCSprite*)mEscKeyProp->getChildByTag(kTagEscKeyPromptSprite);
    if (escKeySprite)
    {
        Localizer::LocaleType locale = getLocale();
        const char* spriteFrameName = "menu-esc";
        switch (locale)
        {
            case Localizer::FR:
                spriteFrameName = "menu-esc-FR";
                break;
            case Localizer::CN:
                spriteFrameName = "menu-esc-CN";
                break;
            default:
                break;
        }
        
        CCSpriteFrame* spriteFrame = spriteFrameByName(spriteFrameName);
        if (spriteFrame)
            escKeySprite->setDisplayFrame(spriteFrame);
    }

    // Save scale
    float savedScale = mEscKeyProp->getScaleX();
    mEscKeyProp->setScale(1.0f);

    CCRect escKeyBounds = mEscKeyProp->boundingBox();
    CCPoint nodePos = mBaseSprite->convertToNodeSpace(ccp(getViewportWidth(), getViewportHeight()));
    mEscKeyProp->setPosition(ccp(nodePos.x - 1.5f * escKeyBounds.size.width, nodePos.y - (escKeyBounds.size.height + 1)));
    
    // Restore scale
    mEscKeyProp->setScale(savedScale);
}

void SceneController::onEvent(int evType, void* evData)
{
	if (evType == MenuButton::EV_TYPE_RAISED())
	{
		MenuButton* button = static_cast<MenuButton*>(evData);
		if (button && button->getTag() == kTagPauseButton)
		{
			onPausePressed();
			//resetPauseButton();
		}
	}
    else if (evType == Jukebox::EV_TYPE_JUKEBOX_SONG_ENDED())
    {
        mIsJukeboxWaiting = true;
    }
    else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
		{
            GLubyte tweenedValue = tweener->getTweenedValue();
            switch (tweener->getTag())
            {
                case kTagEscKeyPromptTweenAlpha:
                {
                    if (mEscKeyProp)
                        mEscKeyProp->setOpacityChildren(tweenedValue);
                }
                    break;
            }
		}
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
		{
            if (tweener->getTag() == kTagEscKeyPromptTweenAlpha)
                removeEscKeyPrompt();
			removeFromJuggler(tweener);
            CC_SAFE_RELEASE_NULL(tweener);
		}
	}
    else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
            float tweenedValue = tweener->getTweenedValue();
            switch (tweener->getTag())
            {
                case kTagEscKeyPromptTweenScale:
                {
                    if (mEscKeyProp)
                        mEscKeyProp->setScale(tweenedValue);
                }
                    break;
            }
		}
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
            if (tweener->getTag() == kTagEscKeyPromptTweenAlpha)
                removeEscKeyPrompt();
			removeFromJuggler(tweener);
            CC_SAFE_RELEASE_NULL(tweener);
		}
	}
}

const char* SceneController::getGameModeSuffix(void)
{
    return MODE_8x6 ? "8x6" : "10x8";
}

bool SceneController::isFullscreen(void) const
{
    return BridgingUtility::isFullscreen();
}

void SceneController::setFullscreen(bool value)
{
    BridgingUtility::setFullscreen(value);
    setUserDefault(CMSettings::B_FULLSCREEN, value);
}

bool SceneController::shouldPromptForRating(void)
{
    return BridgingUtility::shouldPromptForRating();
}

void SceneController::userRespondedToRatingPrompt(bool value)
{
    BridgingUtility::userRespondedToRatingPrompt(value);
}

bool SceneController::isCloudSupported(void)
{
    return CloudWrapper::isCloudSupported();
}

void SceneController::enableCloud(bool enable)
{
    if (!mUserEnabledCloud) // Don't interrupt manual setting
    {
        CloudWrapper::setCloudApproved(enable);
        CloudWrapper::enableCloud(enable);
    }
}

void SceneController::userEnabledCloud(void)
{
    if (!mUserEnabledCloud)
    {
        mUserEnabledCloud = true;
        CloudWrapper::setCloudApproved(true);
        CloudWrapper::enableCloud(true);
    }
}

void SceneController::cloudInitDidComplete(bool enabled)
{
    mUserEnabledCloud = false;
}

bool SceneController::getUserDefaultBool(const char* key, bool defaultValue)
{
	return CCUserDefault::sharedUserDefault()->getBoolForKey(key, defaultValue);
}

void SceneController::setUserDefault(const char* key, bool value)
{
	if (getUserDefaultBool(key) != value)
	{
		mIsUserDefaultsDirty = true;
		CCUserDefault::sharedUserDefault()->setBoolForKey(key, value);
	}
}

int SceneController::getUserDefaultInt(const char* key, int defaultValue)
{
	return CCUserDefault::sharedUserDefault()->getIntegerForKey(key, defaultValue);
}

void SceneController::setUserDefault(const char* key, int value)
{
	if (getUserDefaultInt(key) != value)
	{
		mIsUserDefaultsDirty = true;
		CCUserDefault::sharedUserDefault()->setIntegerForKey(key, value);
	}
}

std::string SceneController::getUserDefaultString(const char* key, const char* defaultValue)
{
    return CCUserDefault::sharedUserDefault()->getStringForKey(key, defaultValue);
}

void SceneController::setUserDefault(const char* key, const char* value)
{
    if (getUserDefaultString(key) != value)
	{
		mIsUserDefaultsDirty = true;
		CCUserDefault::sharedUserDefault()->setStringForKey(key, value);
	}
}

void SceneController::flushUserDefaults(void)
{
	if (mIsUserDefaultsDirty)
	{
		CCUserDefault::sharedUserDefault()->flush();
		mIsUserDefaultsDirty = false;
	}
}

void SceneController::exitApp(void)
{
    flushUserDefaults();
    GameController::GC()->exitApp();
}

