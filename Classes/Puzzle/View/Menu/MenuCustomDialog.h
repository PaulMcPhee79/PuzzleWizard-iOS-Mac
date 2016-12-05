#ifndef __MENU_CUSTOM_DIALOG_H__
#define __MENU_CUSTOM_DIALOG_H__

#include "cocos2d.h"
#include <Puzzle/View/Menu/MenuDialog.h>
#include <Utils/Localizer.h>
class LevelIcon;
class FloatTweener;
class ByteTweener;
class CroppedProp;
class ParticleProp;
USING_NS_CC;

class MenuCustomDialog : public MenuDialog
{
public:
	enum CustomDialogType { LEVEL_UNLOCKED, WIZARD_UNLOCKED, LEVEL_COMPLETE, WIZARD, CREDITS };
    
	MenuCustomDialog(int category, int priority, int levelIndex, uint inputFocus, CustomDialogType type);
	virtual ~MenuCustomDialog(void);

	static MenuCustomDialog* create(int category, int priority, int levelIndex, uint inputFocus,
                                    IEventListener* listener = NULL, CustomDialogType type = WIZARD, bool autorelease = true);
	virtual bool init(void);
    virtual void show(AnimState anim = ANIM_NONE);
    
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
    
    // CustomDialogType::CREDITS
    CCRect getScrollBounds(void) const { return mScrollBounds; }
    void setScrollBounds(const CCRect& value) { mScrollBounds = value; }
    void addScrollingItem(CCNode* item);
	void removeScrollingItem(CCNode* item);
    
    virtual void resolutionDidChange(void);
    virtual void onEvent(int evType, void* evData);
    virtual void update(int controllerState);
	virtual void advanceTime(float dt);

	CustomDialogType getType(void) const { return mType; }

private:
    // Common
    CustomDialogType mType;
    int mLevelIndex;
    std::vector<CCSprite*> mStars;
    
    // CustomDialogType::LEVEL_UNLOCKED
    void showArrowOverTime(float duration, float delay);
    void dropLockOverTime(float duration, float delay);
    
    int mSolvedCount;
    int mSolvedGlowDir;
    CCSprite* mLevelArrow;
    CCSprite* mLevelLock;
    CCSprite* mLevelUnlocked;
    std::vector<CCSprite*>* mSolvedKeys;
    std::vector<LevelIcon*>* mLevelIcons;
    std::vector<FloatTweener*>* mTweenersX;
    std::vector<FloatTweener*>* mTweenersY;
    std::vector<FloatTweener*>* mTweenersRotation;
    std::vector<FloatTweener*>* mTweenersScale;
    ByteTweener* mTweenerArrowAlpha;
    FloatTweener* mTweenerArrowX;
    FloatTweener* mTweenerLockY;
    ByteTweener* mTweenerGlow;
    
    // CustomDialogType::WIZARD_UNLOCKED
    int mTotalSolvedCount;
    int mDropLockFrameCountdown;
    CCSprite* mTotalSolvedKey;
    CCLabelBMFont* mTotalSolvedLabel;
    
    // CustomDialogType::LEVEL_COMPLETE
public:
    // Retro-fit-hack
    static const int kLevelCompleteLabelTagA = 0x1234;
    static const int kLevelCompleteLabelTagB = 0x1235;
    void repositionLevelCompleteLabels(void);
private:
    CCLabelBMFont* mLevelHeader;
    Localizer::LocaleType localeCache;

    // CustomDialogType::WIZARD
    void refreshConfettiSettings(void);
    void randomizeFireworkPosition(ParticleProp* firework);
    
    bool rndToggle;
	int mRGBIncrements[3];
	int mRGB[3];
	float mStarAngle;
	CCNode* mStarsContainer;
    ParticleProp* mConfetti;
    std::vector<unsigned int> mFireworkSoundIds;
    std::vector<ParticleProp*>* mFireworks;
    
    // CustomDialogType::CREDITS
    void recalculateViewableRegion(void);
    bool isTouchEnabled(void);
    void applyTouchTrailMomentum(void);
    void resetTouchTrail(void);
    void setNextTouchTrail(float value);
    void onTouch(int evType, void* evData);
    static const int kTouchTrailLen = 30;
    bool mIsTouched;
    int mTrailIndex;
    float mTouchDelta;
    float mTouchTrail[kTouchTrailLen];
    float mScrollRate;
    float mScrollRateDelta;
    CCRect mScrollContentBounds;
    CCRect mScrollBounds;
    CCNode* mScrollContainer;
    CroppedProp* mScrollCropper;
};
#endif // __MENU_CUSTOM_DIALOG_H__
