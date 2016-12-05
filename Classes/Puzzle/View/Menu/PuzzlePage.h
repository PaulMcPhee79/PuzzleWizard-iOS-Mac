#ifndef __PUZZLE_PAGE_H__
#define __PUZZLE_PAGE_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Interfaces/ILocalizable.h>
#include <Utils/LangFX.h>
class PuzzlePageEntry;
class Puzzle;
USING_NS_CC;

class PuzzlePage : public Prop, public ILocalizable
{
public:
	static const int kNumPuzzlesPerPage = 6;
	static const int kNumPuzzlesPerRow = 3;
	static const int kNumPuzzlesPerColumn = 2;

	PuzzlePage(void);
	virtual ~PuzzlePage(void);

	virtual bool init(void);
    
    void resolutionDidChange(float parentScale);
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);

    void refreshColorScheme(void);
	PuzzlePageEntry* getEntry(int index) const;
	int getNumPuzzles(void) const { return mNumPuzzlesCache; }
	int indexAtPoint(const CCPoint& pt, CCNode* space) const;
	void setHeaderLabel(CCLabelBMFont* headerLabel);
	void setPuzzleAtIndex(int index, Puzzle* puzzle);
	void setLockedAtIndex(int index, bool locked);
	void setSolvedAtIndex(int index, bool solved);
	void setHighlightColor(uint color);
	void highlightPuzzle(int index, bool enable);
	void clear();
    
    static LangFX::PuzzlePageSettings getSettings(void);

private:
	static const int kNumLayers = 3;

    static void setSettings(const LangFX::PuzzlePageSettings& settings);
	void setNumPuzzles(int value) { mNumPuzzlesCache = value; }
	void refreshNumPopulatedPuzzles(void);
    void positionHeaderLabel(CCLabelBMFont* headerLabel);

	int mNumPuzzlesCache;
    CCLabelBMFont* mHeaderLabel; // Weak pointer
	PuzzlePageEntry* mPuzzles[kNumPuzzlesPerPage];
	CCNode* mLayers[kNumLayers];
};
#endif // __PUZZLE_PAGE_H__
