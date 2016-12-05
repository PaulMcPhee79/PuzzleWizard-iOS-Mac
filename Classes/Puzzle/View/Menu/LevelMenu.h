#ifndef __LEVEL_MENU_H__
#define __LEVEL_MENU_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/IInteractable.h>
#include <Interfaces/IResDependent.h>
#include <Interfaces/ILocalizable.h>
#include <Utils/CMTypes.h>
class CMSprite;
class MenuButton;
class IconButton;
class CroppedProp;
class LevelIcon;
class PuzzlePage;
class LevelOverlay;
class Level;
class Puzzle;
class FloatTweener;
class ByteTweener;
class RateIcon;
USING_NS_CC;

class LevelMenu : public Prop, public EventDispatcher, public IEventListener, public IInteractable,
				  public IResDependent, public ILocalizable, public CCKeypadDelegate
{
public:
	static const int kNumLevels = 12;
	static const char* kLevelNames[];
	static const char* kLevelTextureNames[];
	static const uint kLevelColors[];

	static int EV_TYPE_DID_TRANSITION_IN() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_DID_TRANSITION_OUT() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_PUZZLE_SELECTED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_DEVICE_BACK_CLICKED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    static int EV_TYPE_RATE_THE_GAME() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	LevelMenu(int category, CCArray* levels);
	virtual ~LevelMenu(void);

	virtual bool init(void);
	virtual void onEnter();
    virtual void resolutionDidChange(void);
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
    
    void refresh(void);

	bool getUnlockedAll(void) const { return mUnlockedAll; }
	void setUnlockedAll(bool value)
	{
		mUnlockedAll = value;
		refreshLevelLocks();
        refreshPuzzleLocks();
	}
	int getSelectedPuzzleID(void);
	int getNumLevels(void) const;
	Level* getSelectedLevel(void);
	Puzzle* getSelectedPuzzle(void);
	int getLevelIndex(void) const { return mLevelIndex; }
	void setLevelIndex(int value);
	int getPuzzleIndex(void) const { return mPuzzleIndex; }
	void setPuzzleIndex(int value);

    void enableRateTheGameIcon(bool enable);
    void animateRateTheGameIcon(bool enable);
    void refreshColorScheme(void);
	void jumpToLevelIndex(int levelIndex, int puzzleIndex);
	void returnToLevelMenu(void);

	void showOverTime(float duration);
	void hideOverTime(float duration);
    void hideInstantaneously(void);
    
    virtual unsigned int getInputFocus(void) const { return CMInputs::HAS_FOCUS_MENU; }
    virtual void update(int controllerState);

    // Android system key events
    virtual void keyBackClicked();
    virtual void keyMenuClicked() {}

	virtual void onEvent(int evType, void* evData);
	virtual void advanceTime(float dt);

private:
	enum LevelMenuState { IDLE = 0, IDLE_2_PUZZLES, IDLE_2_LEVELS, LEVELS, LEVELS_2_PUZZLES, PUZZLES, PUZZLES_2_LEVELS, PUZZLES_2_IDLE };
	
    static const float kLevel2PuzzleTransitionDuration;
    static const float kPuzzle2LevelTransitionDuration;
    static const float kPuzzle2IdleTransitionDuration;

    void createLevelMenuHeader(void);
    void layoutLevelMenuHeader(void);
    float calcMaxScale(void);
    CCRect calcCanvasViewableRegion(void);
    void repositionBackButton(void);
    void repositionRateIcon(void);
	LevelMenuState getState(void) const { return mState; }
	void setState(LevelMenuState value);
    bool isScrolling(void) const { return mIsScrolling; }
    void setIsScrolling(bool value);
    Coord getRepeatVec(void) const { return mRepeatVec; }
    void setRepeatVec(Coord value)
    {
        if (mRepeatVec != value)
        {
            mRepeatVec = value;
            mRepeatCounter = 2 * mRepeatDelay;
        }
    }
    float getRepeatCounter(void) const { return mRepeatCounter; }
    void setRepeatCounter(float value) { mRepeatCounter = value; }
    float getRepeatDelay(void) const { return mRepeatDelay; }
    void setRepeatDelay(float value) { mRepeatDelay = value; }
    bool isBackButtonHighlighted(void) const;
    void enableBackButtonHighlight(bool value);
    
    void updateCrashContext(const char* value, const char* key);
    void capFramerate(void);
    const char* getLevelName() const;
	void setPageIndex(int value);
	void scrollContentBy(float x);
	void scrollContentTo(float x);
	void setContentX(float x);
	bool isLevelUnlocked(int levelIndex);
	bool isPuzzleUnlocked(int levelIndex, int puzzleIndex);
	void showHelpUnlock(int levelIndex);
    CCRect getLevelSpriteViewableRect(LevelIcon* icon, CCRect& iconBounds);
	void refreshArrowVisibility(void);
	void refreshLevelSpriteVisibility(void);
    void refreshLevelSpriteOpacity(void);
	void refreshLevelLocks(int levelIndex = -1);
	void refreshLevelsSolved(int levelIndex = -1);
	void refreshPuzzleLocks(void);
	void refreshPuzzlesSolved(void);
	void highlightPuzzle(int index, bool enable);
	void populatePuzzlePage(CCArray* puzzles);
	void selectCurrentLevel(void);
	void selectCurrentPuzzle(void);
	void transitionLevels2Puzzles(float duration = kLevel2PuzzleTransitionDuration);
	void transitionPuzzles2Levels(float duration = kPuzzle2LevelTransitionDuration);
	void transitionPuzzles2Idle(float duration = kPuzzle2IdleTransitionDuration);
    void processNavInput(Coord moveVec);
	void onTouch(int evType, void* evData);

	LevelMenuState mState;
	LevelMenuState mQueuedState;
    
	bool mIsTouchEnabled;
	bool mIsTouched;
	bool mIsArrowTouched;
	bool mDidTouchSelect;
	bool mDidTouchMove;
    bool mDidTouchCancel;
    bool mIsScrolling;
	bool mUnlockedAll;
    
    Coord mRepeatVec;
    float mRepeatCounter;
    float mRepeatDelay;

	CCSprite* mHelpSprite;
	CCLabelBMFont* mHelpLabel;
	Prop* mHelpContainer;
	ByteTweener* mHelpOpacityTweener;

    CCSprite* mHeaderQuad;
    CCSprite* mHeaderSprite;
    CCSprite* mFooterKey;
	CCLabelBMFont* mFooterLabel;
	MenuButton* mPrevPageArrow;
	MenuButton* mNextPageArrow;
	Prop* mDecorations;
    
    MenuButton* mBgSelectButton;
    
    LevelOverlay* mLevelOverlay;
    Prop* mOverlayContainer;
    
    RateIcon* mRateIcon;

	IconButton* mBackButton;

	Prop* mContent;
    Prop* mContentShadows;
	CroppedProp* mCanvas;

	CMSprite* mBgCurtain;

	int mLevelIndex;
	CCArray* mLevels;
	std::vector<LevelIcon*> mLevelIcons;
    std::vector<CCSprite*> mLevelIconShadows;
	std::vector<CCLabelBMFont*> mLevelHeaders;

	int mPuzzleIndex;
	PuzzlePage* mPuzzlePage;

	int mPageIndex;
	int mScrollDir;
	float mScrollDelta;
	float mScrollDeltaAbs;
	FloatTweener* mScrollTweener;

	ByteTweener* mContentOpacityTweener;
	ByteTweener* mDecorationsOpacityTweener;
	ByteTweener* mPuzzlePageOpacityTweener;
	ByteTweener* mBgOpacityTweener;
};
#endif // __LEVEL_MENU_H__
