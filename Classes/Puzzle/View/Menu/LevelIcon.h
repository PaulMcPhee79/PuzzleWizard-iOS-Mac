#ifndef __LEVEL_ICON_H__
#define __LEVEL_ICON_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Interfaces/ILocalizable.h>
USING_NS_CC;

class LevelIcon : public Prop, public ILocalizable
{
public:
	LevelIcon(int category, int levelIndex, int levelID);
	virtual ~LevelIcon(void);

	virtual bool init(void);

	bool isLocked(void) const { return mLock && mLock->isVisible(); }
	void setLocked(bool value) { if (mLock) mLock->setVisible(value); }
	int getLevelID(void) const { return mLevelID; }
    CCRect getVisibleBounds(void);
    CCPoint getVisibleWorldCenter(void);
    CCPoint getSolvedKeyWorldCenter(void);
    
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
    
	void enableHighlight(bool enable);
	void setPuzzlesSolved(int numSolved, int numPuzzles);
	void setLevelTextColor(uint color);
	void setPuzzlesSolvedColor(uint color);
    
    void enableSolvedGlow(bool enable);
    GLubyte getSolvedGlowOpacity(void) const { return mSolvedGlow ? mSolvedGlow->getOpacity() : 0; }
    void setSolvedGlowOpacity(GLubyte value) { if (mSolvedGlow) mSolvedGlow->setOpacity(value); }

private:
    void layoutLabel(void);
    
	int mLevelIndex;
	int mLevelID;
    float mLabelBaseScale;
    
	CCLabelBMFont* mLevelLabel;
    CCLabelBMFont* mSolvedLabel;
    
    CCSprite* mBorder;
	CCSprite* mHighlight;
    CCSprite* mSolvedIcon;
    CCSprite* mSolvedGlow;
	CCSprite* mLock;
	CCSpriteBatchNode* mBatch;
};
#endif // __LEVEL_ICON_H__
