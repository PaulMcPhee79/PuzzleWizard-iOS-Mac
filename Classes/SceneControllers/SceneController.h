#ifndef __SCENE_CONTROLLER_H__
#define __SCENE_CONTROLLER_H__

#ifdef __APPLE__
	#define CM_TEX_EXT ".png"
	#define CM_TEX_ZIP_EXT ".png" // ".pvr.ccz"
	#define CM_IMG_EXT ".png" // ".pvr.ccz"
	#define CM_IMG_FONT_EXT ".png"
	#define CM_SND_EXT ".caf"
	#define CM_MUSIC_EXT ".mp3" // ".aifc"
	#define CM_FONT_EXT ".fnt"
#elif defined (__ANDROID__)
	#define CM_TEX_EXT ".png"
	#define CM_TEX_ZIP_EXT ".png"
	#define CM_IMG_EXT ".png"
	#define CM_IMG_FONT_EXT ".png"
	#define CM_SND_EXT ".ogg" //".3gp"
	#define CM_MUSIC_EXT ".mp3" //".ogg"
	#define CM_FONT_EXT ".fnt"
#else
	#define CM_TEX_EXT ".png"
	#define CM_TEX_ZIP_EXT ".png"
	#define CM_IMG_EXT ".png"
	#define CM_IMG_FONT_EXT ".png"
	#define CM_SND_EXT ".wav"
	#define CM_MUSIC_EXT ".wav"
	#define CM_FONT_EXT ".fnt"
#endif

#define CM_MUTE 0xFEEDBAB // Return value indicating sound engine is muted or non functioning (see CocosDension.h)

#include "cocos2d.h"
#include <Managers/SpriteLayerManager.h>
#include <Interfaces/IEventListener.h>
#include <Extensions/Juggler.h>
#include <Managers/TouchTypes.h>
#include <Managers/InputTypes.h>
#include <Managers/GameSettings.h>
#include <Utils/CMTypes.h>
#include <Utils/DeviceDetails.h>
#include <Interfaces/IInteractable.h>
#include <Puzzle/View/PuzzleHelper.h>
#include <Utils/TextUtils.h>
#include <Utils/Localizer.h>
#include <Interfaces/ILocalizable.h>
class Prop;
class PuzzleBoard;
class TouchManager;
class Jukebox;
class IResDependent;
USING_NS_CC;

class SceneController : public CCObject, public IEventListener
{
public:
	static const float kDefaultSceneWidth;
	static const float kDefaultSceneHeight;

	SceneController(void);
	virtual ~SceneController(void);

	virtual void willGainSceneFocus(void);
	virtual void willLoseSceneFocus(void);
	virtual void attachEventListeners(void) { detachEventListeners(); } // Make sure base classes aren't adding listeners more than once
	virtual void detachEventListeners(void) { }
	virtual void applyGameSettings(void);
	virtual void presentScene(void);
	virtual void enableMenuMode(bool enable) { }
	virtual void showMenuDialog(const char* key) { }
    virtual void showEscDialog(void) { }
    virtual void hideEscDialog(void) { /* resetPauseButton(); */ }
	virtual void enableGodMode(bool enable) { }
    virtual void reportBuggedPuzzle() { }
    virtual void showBetaIQRatingDialog() { }
    
    virtual void beginContentLoad(void) = 0;
    virtual void loadNextContent(void) = 0;
    virtual bool isContentLoaded(void) = 0;
    virtual float getContentLoadProgress(void) = 0;
    
    virtual void applicationDidEnterBackground(void);
    virtual void applicationWillEnterForeground(void);
    virtual void applicationWillTerminate(void);
    virtual void splashViewDidHide(void);
    virtual void invalidateStateCaches(void);

	virtual void onEvent(int evType, void* evData);

	bool getIsScenePaused(void) const { return mIsScenePaused; }
	void setIsScenePaused(bool value) { mIsScenePaused = value; }
	virtual int getPauseCategory(void) const { return 0; }
    virtual float getSetupProgress(void) { return 0; }
    double getFps(void) const;
	float getViewWidth(void) const { return mViewWidth; }
	float getViewHeight(void) const { return mViewHeight; }
	float getScaledViewWidth(void) const { return mViewWidth * getViewScale().x; }
	float getScaledViewHeight(void) const { return mViewHeight * getViewScale().y; }
	float getFullscreenWidth(void) const { return getViewportWidth() * getInvViewScale().x; }
	float getFullscreenHeight(void) const { return getViewportHeight() * getInvViewScale().y; }
	float getViewportWidth(void) const;
	float getViewportHeight(void) const;
    float getViewportScaledWidth(void) const;
    float getViewportScaledHeight(void) const;
	vec2 getViewScale(void) const;
	vec2 getInvViewScale(void) const;
	float getViewAspectRatio(void) const;
	float getViewportAspectRatio(void) const;
	float getMaximizingContentScaleFactor(void) const;
	CCPoint getContentOffset(void) const;
	const std::string& getFontName(int fontSize);
	float getTimeDelta(void) const { return mTimeDelta; }
	Juggler* getJuggler(void) { return mJuggler; }
    CCScene* getCCScene(void) const { return mScene; }
    
    float getPuzzleDuration(void) const { return mPuzzleDuration; }
    
    Puzzles::ColorScheme getColorScheme(void) const { return Puzzles::getColorScheme(); }
    virtual void setColorScheme(Puzzles::ColorScheme value);
    
    virtual void enableLowPowerMode(bool enable);

    void registerResDependent(IResDependent* resDependent);
    void deregisterResDependent(IResDependent* resDependent);
    virtual void resolutionDidChange(int width, int height);
    
    void registerLocalizable(ILocalizable* localizable);
    void deregisterLocalizable(ILocalizable* localizable);
    virtual void localeDidChange(void);
    
	void registerForTouches(CMTouches::TouchCategory category, IEventListener* listener);
	void deregisterForTouches(CMTouches::TouchCategory category, IEventListener* listener);
    
    void subscribeToInputUpdates(IInteractable* client, bool modal = false);
    void unsubscribeToInputUpdates(IInteractable* client, bool modal = false);
    bool hasInputFocus(uint focus);
    void pushFocusState(uint focusState, bool modal = false);
    void popFocusState(uint focusState = CMInputs::FOCUS_STATE_NONE, bool modal = false);
    void popToFocusState(uint focusState, bool modal = false);

	void addToJuggler(IAnimatable* obj);
	void removeFromJuggler(IAnimatable* obj);
	void removeTweensWithTarget(void* target);

	CCTexture2D* textureByName(const char* name) const;
	CCSpriteFrame* spriteFrameByName(const char* name) const;
	CCArray* spriteFramesStartingWith(const char* name) const;

	GLint uniformLocationByName(const char* name);
	void setUniformLocation(const char* name, GLint uniformLoc);
	void purgeUniformLocations(void);
	GLuint textureUnitById(GLuint textureId);
	void setTextureUnit(GLuint textureId, GLuint textureUnit);
    
    Localizer::LocaleType getLocale(void) const { return Localizer::getLocale(); }
    void setLocale(Localizer::LocaleType locale);
    static std::string localizeString(const char* str) { return Localizer::localizeString(str); }
    static std::string localizeString(const std::string& str) { return Localizer::localizeString(str); }

    virtual void abortCurrentPuzzle(void) { }
    virtual void resetCurrentPuzzle(void) { }
	virtual void returnToPuzzleMenu(void) { }
	virtual void returnToLevelMenu(void) { }
    void resetPauseButton(void);

	void enableSfx(bool enable);
	bool isSfxEnabled(void) { return mIsSfxEnabled; }
	void enableMusic(bool enable);
	bool isMusicEnabled(void) { return mIsMusicEnabled; }
    bool isSfxMuted(void) const { return mIsSfxMuted; }
    void muteSfx(bool value) { mIsSfxMuted = value; }
    
    int getSfxVolume(void) const;
    void setSfxVolume(int value);
    int getMusicVolume(void) const;
    void setMusicVolume(int value);

	unsigned int playSound(std::string name);
    void stopSound(unsigned int soundId);
	void playMusic(std::string name, bool loop = false);
    
    std::string getCurrentJukeboxSongName(void) const;
    std::string getPreviousJukeboxSongName(void) const;
    std::string playPrevJukeboxSong(bool loop = false);
    std::string playNextJukeboxSong(bool loop = false);
    
	virtual void advanceTime(float dt);

	void addProp(Prop* prop);
	void removeProp(Prop* prop, bool tryCache = false);

	void addPauseButton(const char* path);
	void removePauseButton(void);
    
    void addEscKeyPrompt(void);
    void removeEscKeyPrompt(void);
    void showEscKeyPrompt(float duration, float delay);
    void updateEscKeyPrompt(void);
    
    void addFontName(const char* fontName, int fontSize);
    void removeFontName(int fontSize);
    
    const char* getGameModeSuffix(void);
    
    bool isFullscreen(void) const;
    void setFullscreen(bool value);
    
    virtual bool shouldPromptForRating(void);
    virtual void userRespondedToRatingPrompt(bool value);
    
    bool isCloudSupported(void);
    virtual void enableCloud(bool enable);
    virtual void userEnabledCloud(void);
    virtual void cloudInitDidComplete(bool enabled);
    
    bool isRestoringPreviousSession(void) const { return mIsRestoringPreviousSession; }

    float getBaseScale(void) const;
    Prop* getBaseSprite(void) const { return mBaseSprite; }
	static bool getUserDefaultBool(const char* key, bool defaultValue = false);
	static void setUserDefault(const char* key, bool value);
	static int getUserDefaultInt(const char* key, int defaultValue = 0);
	static void setUserDefault(const char* key, int value);
    static std::string getUserDefaultString(const char* key, const char* defaultValue = "");
	static void setUserDefault(const char* key, const char* value);
	static void flushUserDefaults(void);
    void exitApp(void);
    
protected:
	static const int kTagPauseButton = 1001;

    virtual void setupController(void) { }
    virtual void preSetupController(void);
	virtual void reconfigureBaseSprite(void);
	virtual void setupPrograms(void) { }
	virtual void onPausePressed(void) { }
    void setResumeMusicAttempts(int value);
    void resumeMusicFromSystemPause(float dt);
    void setPuzzleDuration(float value) { mPuzzleDuration = MAX(0, value); }
    void setRestoringPreviousSession(bool value) { mIsRestoringPreviousSession = value; }

	bool mLocked;
	bool mDestructLock;
	bool mIsScenePaused;
    bool mIsSfxMuted;
    bool mIsRestoringPreviousSession;
    bool mUserEnabledCloud;
	
	std::string mSceneKey;
	CCArray* mProps;
	CCArray* mAdvProps;
	CCArray* mPropsAddQueue;
	CCArray* mPropsRemoveQueue;

	Juggler* mJuggler;

	Prop* mBaseSprite;
	CCScene* mScene;
	SpriteLayerManager* mSpriteLayerManager;
	TouchManager* mTouchManager;
    
    bool mIsJukeboxWaiting;
    Jukebox* mJukebox;

private:
	void addQueuedProps();
	void removeQueuedProps();
    void updatePauseButton(void);

	bool mIsSfxEnabled;
	bool mIsMusicEnabled;
    bool mWasMusicPlaying;
	float mViewWidth;
    float mViewHeight;
	float mTimeDelta;
    float mPuzzleDuration;
    
    // Resuming music can fail, so allow for timing issues by retrying X times.
    int mNumResumeMusicAttempts;
    float mResumeMusicInterval;

	std::map<int, std::string> mFontNames;

	std::map<std::string, GLint> mUniformLocations;
	std::map<GLuint, GLuint> mTextureUnits; // <textureId, textureUnit>
    
    std::vector<IResDependent*> mResDependents;
    std::vector<ILocalizable*> mLocalizables;

	Prop* mPauseProp;
    Prop* mEscKeyProp;
};
#endif // __SCENE_CONTROLLER_H__
