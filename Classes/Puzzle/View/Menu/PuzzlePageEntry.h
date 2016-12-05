#ifndef __PUZZLE_PAGE_ENTRY_H__
#define __PUZZLE_PAGE_ENTRY_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
class PuzzleBoard;
class TileDecoration;
USING_NS_CC;

class PuzzlePageEntry : public Prop, public ILocalizable
{
public:
	PuzzlePageEntry(PuzzleBoard* board);
	virtual ~PuzzlePageEntry(void);

	virtual bool init(void);

	CCPoint getMinPoint(void);
	bool isPopulated(void) const { return mIsPopulated; }
	void setIsPopulated(bool value) { setVisible(value); mIsPopulated = value; }
	bool isLocked(void) const { return mIsLocked; }
	void setIsLocked(bool value) { mIsLocked = value; if (mLock) mLock->setVisible(value); }
	bool isSolved(void) const { return mIsSolved; }
	void setIsSolved(bool value) { mIsSolved = value; if (mSolved) mSolved->setVisible(value); }
	bool isHighlighted(void) const { return mIsHighlighted; }
	void setIsHighlighted(bool value);
	uint getHighlightColor(void) const { return mHighlightColor; }
	void setHighlightColor(uint value);
	PuzzleBoard* getPuzzleBoard(void) const { return mBoard; }
    CCNode* getCenteredNode(void) const { return mLock; } // For easier positioning in PuzzlePage
	void setPuzzleBoard(PuzzleBoard* value) { mBoard = value; } // PuzzlePage will manage lifetime.
	void resizeBoard(void);
	const char* getString(void);
	void setString(const char* value);
    void setFntFile(const char* value);
    
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);

private:
    void resizeLabel(void);
    void repositionHighlightedIcon(void);
    CCSize getShrunkLabelSizeDiff(void);
    
	bool mIsPopulated;
	bool mIsLocked;
	bool mIsSolved;
	bool mIsHighlighted;
    bool mIsLabelShrunkToFit;
	uint mHighlightColor;
    float mHighlightOffsetY;
    float mLabelBaseScale;
    float mLabelWidthMax;

	CCSprite* mLock;
	CCSprite* mSolved;
    CCNode* mSolvedContainer;
    TileDecoration* mHighlightedIcon;
	PuzzleBoard* mBoard;
    CCSprite* mBoardShadow;
    CCSprite* mIqTagShadow;
	CCNode* mBoardContainer;
	CCLabelBMFont* mLabel;
};
#endif // __PUZZLE_PAGE_ENTRY_H__
