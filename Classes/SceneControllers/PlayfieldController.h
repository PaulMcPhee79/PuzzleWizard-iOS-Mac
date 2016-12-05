#ifndef __PLAYFIELD_CONTROLLER_H__
#define __PLAYFIELD_CONTROLLER_H__

#include "cocos2d.h"
#include "SceneController.h"
#include <Interfaces/IInteractable.h>
#include <Managers/ControlsManager.h>
class PlayfieldView;
class PuzzleController;
class MenuController;
class FpsSampler;
class Puzzle;
USING_NS_CC;

class PlayfieldController : public SceneController, public IInteractable
{
public:
	enum PfState { HIBERNATING = 0, TITLE, MENU, PLAYING };

	PlayfieldController(void);
	virtual ~PlayfieldController(void);

	virtual void willGainSceneFocus(void);
	virtual void willLoseSceneFocus(void);
	virtual void attachEventListeners(void);
	virtual void detachEventListeners(void);
	virtual void applyGameSettings(void);
	virtual void enableMenuMode(bool enable);
	virtual void showMenuDialog(const char* key);
    virtual void showEscDialog(void);
    virtual void hideEscDialog(void);
	virtual void enableGodMode(bool enable);
    virtual void reportBuggedPuzzle();
    virtual void showBetaIQRatingDialog();
    virtual void setColorScheme(Puzzles::ColorScheme value);
    virtual void enableLowPowerMode(bool enable);
    
    virtual void beginContentLoad(void);
    virtual void loadNextContent(void);
    virtual bool isContentLoaded(void);
    virtual float getContentLoadProgress(void);
	
    virtual void applicationDidEnterBackground(void);
    virtual void applicationWillEnterForeground(void);
    virtual void applicationWillTerminate(void);
    virtual void splashViewDidHide(void);
    
    virtual unsigned int getInputFocus(void) const { return CMInputs::HAS_FOCUS_ALL; }
    virtual void update(int controllerState);
    
    void beginFpsSample(void);
    virtual void invalidateStateCaches(void);
    virtual void resolutionDidChange(int width, int height);
	virtual void onEvent(int evType, void* evData);

    virtual void abortCurrentPuzzle(void);
    virtual void resetCurrentPuzzle(void);
	virtual void returnToPuzzleMenu(void);
	virtual void returnToLevelMenu(void);
    
    virtual void userRespondedToRatingPrompt(bool value);
    virtual void enableCloud(bool enable);
    virtual void cloudInitDidComplete(bool enabled);

	virtual void advanceTime(float dt);
	PfState getState(void) const { return mState; }
	PuzzleController* getPuzzleController(void) { return mPuzzleController; }

	void proceedToNextUnsolvedPuzzle(void);
	void puzzleWasSelectedAtMenu(int puzzleID);

	virtual int getPauseCategory(void) const;
	virtual void onPausePressed(void);

protected:
    virtual void setupController(void);
    virtual void preSetupController(void);
	void setupReusables(void);
	virtual void setupPrograms(bool reload);
	void setState(PfState value);
    void setIdleTimerDisabled(bool disabled);
    
private:
    // Logging utilities
    const char* puzzleDuration2Str(float duration);
    std::string puzzleIndex2LevelSpanStr(int levelIndex);
    std::string getEventPuzzleName(const Puzzle& puzzle);
    std::string getEventPuzzleDuration(const Puzzle& puzzle, float duration);
    void logPuzzleAnalyticsEvent(bool didSolve);
    
    void levelWasUnlocked(int levelIndex);
    void levelWasCompleted(int levelIndex);
    void resetCurrentPuzzle_Internal(void);
    void serializeCurrentPuzzle(void);
    void generateMipMaps(void);
    void finishFpsSample(void);
    void processMilestoneDialogs(void);
    void setCloudEvent(uint evCode, bool value);
    void clearCloudEvents(void);
    void processCloudEvents(void);
    void refreshAfterProgressChanged(void);

    bool mHasSetupCompleted;
    bool mIgnoreNewPuzzleEvents;
    bool mHasProcessedSolvedPuzzle;
    bool mHasSampledFps;
    bool mHasReportedBuggedPuzzle;
    bool mIsCloudEnableEnqueued;
    bool mIsIdleTimerDisabled;
    uint mCloudEvents; // iCloud events bitmap
	float mMiscTimer;
	PfState mState;
	PfState mPrevState;
    FpsSampler* mFpsSampler;
	PuzzleController* mPuzzleController;
	PlayfieldView* mView;
	MenuController* mMenuController;
    
    // Content loading
    enum ContentType { IMAGE, ATLAS, FONT, AUDIO, PRE_SETUP, PROGRAMS, REUSABLES, SETUP, PAUSE };
    struct ContentItem
    {
        ContentItem(ContentType type, int weighting, int fontSize, std::vector<std::string> names)
        {
            this->type = type;
            this->weighting = weighting;
            this->fontSize = fontSize;
            this->names = names;
        }
        ContentType type;
        int weighting;
        int fontSize;
        std::vector<std::string> names;
    };
    typedef std::vector<ContentItem> ContentQueue;
    bool mIsContentLoaded;
    int mContentQueueCurrent;
    int mContentQueuePeak;
    std::vector<ContentItem> mContentLoadQueue;
    void enqueueContent(ContentType type, std::string name);
    void enqueueContent(ContentType type, int weighting, std::string name);
};
#endif // __PLAYFIELD_CONTROLLER_H__
