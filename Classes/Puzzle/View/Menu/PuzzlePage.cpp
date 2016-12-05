
#include "PuzzlePage.h"
#include <Puzzle/Data/Puzzle.h>
#include <Puzzle/View/Playfield/PuzzleBoard.h>
#include <Puzzle/View/Menu/PuzzlePageEntry.h>
#include <Utils/Utils.h>
USING_NS_CC;

//static const int kHighlightLayer = 0;
static const int kPuzzleLayer = 1;
static const int kHeaderLayer = 2;
//static const float kPuzzleHorizSeparation = 332.0f; // 332.0f; // 372.0f;
//static const float kPuzzleVertSeparation = 248.0f; // 242.0f;

/*
typedef struct
{
    float puzzleHorizSeparation;
    float puzzleVertSeparation;
    float puzzleBoardWidth;
    float puzzleBoardHeight;
    float puzzleEntryYOffset;
    float headerImageYOffsetFactor;
    float defaultOffsetY;
    
} PuzzlePageSettings;
*/
static LangFX::PuzzlePageSettings s_Settings;

LangFX::PuzzlePageSettings PuzzlePage::getSettings(void)
{
    return s_Settings;
}

void PuzzlePage::setSettings(const LangFX::PuzzlePageSettings& settings)
{
    s_Settings = settings;
}

#pragma warning( disable : 4351 ) // new behavior: elements of array 'xyz' will be default initialized (changed in Visual C++ 2005)
PuzzlePage::PuzzlePage(void)
	:
	mNumPuzzlesCache(0),
    mHeaderLabel(NULL),
	mPuzzles(),
	mLayers()
{
    PuzzlePage::setSettings(LangFX::getPuzzlePageSettings());
}

PuzzlePage::~PuzzlePage(void)
{
    mHeaderLabel = NULL;
	for (int i = 0; i < kNumPuzzlesPerPage; ++i)
		CC_SAFE_RELEASE_NULL(mPuzzles[i]);
	for (int i = 0; i < kNumLayers; ++i)
		CC_SAFE_RELEASE_NULL(mLayers[i]);
}

bool PuzzlePage::init(void)
{
	for (int i = 0; i < kNumLayers; ++i)
	{
		mLayers[i] = new CCNode();
		addChild(mLayers[i]);
	}

    LangFX::PuzzlePageSettings settings = PuzzlePage::getSettings();
	for (int i = 0; i < kNumPuzzlesPerPage; ++i)
	{
		PuzzleBoard* board = new PuzzleBoard(getCategory());
		
		Puzzle* puzzle = new Puzzle(Puzzle::nextPuzzleID(), "", MODE_8x6 ? 8 : 10, MODE_8x6 ? 6 : 8);
		board->initWithPuzzle(puzzle);
		board->enableMenuMode(true);
		board->enableHighlight(false);

		PuzzlePageEntry* puzzleEntry = new PuzzlePageEntry(board);
		puzzleEntry->init();
		puzzleEntry->setPosition(ccp(
                                     (mScene->getViewWidth() - ((kNumPuzzlesPerRow-1) * settings.puzzleHorizSeparation + settings.puzzleBoardWidth)) / 2 +
                                     (i % kNumPuzzlesPerRow) * settings.puzzleHorizSeparation - 8, // Roughly 20 to account for half tile overhang to the left of the puzzleboard origin
                                     mScene->getViewHeight() - (settings.puzzleEntryYOffset + (i / kNumPuzzlesPerRow) * settings.puzzleVertSeparation)));
		mPuzzles[i] = puzzleEntry;
		mLayers[kPuzzleLayer]->addChild(puzzleEntry);
		CC_SAFE_RELEASE(puzzle);
		CC_SAFE_RELEASE(board);

		CCRect bounds = CMUtils::unionRect(boundingBox(), puzzleEntry->boundingBox());
		setContentSize(bounds.size);
	}

	return true; // Obvious when it fails
}

void PuzzlePage::resolutionDidChange(float parentScale)
{
    float viewportWidth = 0.925f * mScene->getViewportWidth();
    if (viewportWidth == 0) // Avoid DBZ
        return;
    
    LangFX::PuzzlePageSettings settings = PuzzlePage::getSettings();
    float pageWidth = (kNumPuzzlesPerRow-1) * settings.puzzleHorizSeparation + settings.puzzleBoardWidth + 48;
    CCPoint worldLeft = convertToWorldSpace(CCPointZero);
    CCPoint worldRight = convertToWorldSpace(ccp(pageWidth, 0.0f));
    pageWidth = worldRight.x - worldLeft.x;
    
    float pageViewportRatio = pageWidth / viewportWidth;
    if (pageViewportRatio == 0) // Avoid DBZ
        return;
    
    float pageScaleX = 1.0f / pageViewportRatio;
    float horizSeparation = MAX(330, MIN(1.0f, pageScaleX) * settings.puzzleHorizSeparation);
    for (int i = 0; i < kNumPuzzlesPerPage; ++i)
    {
        PuzzlePageEntry* puzzleEntry = mPuzzles[i];
        puzzleEntry->resizeBoard();
        puzzleEntry->setPosition(ccp(
                                     (mScene->getViewWidth() - ((kNumPuzzlesPerRow-1) * horizSeparation + settings.puzzleBoardWidth)) / 2 +
                                     (i % kNumPuzzlesPerRow) * horizSeparation - 8, // Roughly 8 to account for half tile overhang to the left of the puzzleboard origin
                                     mScene->getViewHeight() - (settings.puzzleEntryYOffset + (i / kNumPuzzlesPerRow) * settings.puzzleVertSeparation)));
    }
}

void PuzzlePage::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    LangFX::PuzzlePageSettings settings = LangFX::getPuzzlePageSettings();
    PuzzlePage::setSettings(settings);
    
    for (int i = 0; i < kNumPuzzlesPerPage; ++i)
	{
		if (mPuzzles[i])
            mPuzzles[i]->localeDidChange(fontKey, FXFontKey);
	}
}

void PuzzlePage::refreshColorScheme(void)
{
    for (int i = 0; i < kNumPuzzlesPerPage; ++i)
	{
		if (mPuzzles[i])
			mPuzzles[i]->getPuzzleBoard()->refreshColorScheme();
	}
}

PuzzlePageEntry* PuzzlePage::getEntry(int index) const
{
	if (index >= 0 && index < kNumPuzzlesPerPage)
		return mPuzzles[index];
	else
		return NULL;
}

int PuzzlePage::indexAtPoint(const CCPoint& pt, CCNode* space) const
{
	int index = -1;
	CCPoint adjPt = pt;
    LangFX::PuzzlePageSettings settings = PuzzlePage::getSettings();
	for (int i = 0; i < kNumPuzzlesPerPage; ++i)
	{
		if (mPuzzles[i] && mPuzzles[i]->isPopulated())
		{
            CCNode* centerNode = mPuzzles[i]->getCenteredNode();
            if (centerNode)
            {
                CCRect bounds = CMUtils::boundsInSpace(space, centerNode);
                bounds.setRect(
                               bounds.getMidX() - settings.puzzleBoardWidth / 2,
                               bounds.getMidY() - settings.puzzleBoardHeight / 2,
                               settings.puzzleBoardWidth,
                               settings.puzzleBoardHeight);
                
                if (bounds.containsPoint(adjPt))
                {
                    index = i;
                    break;
                }
            }
		}
	}
	return index;
}

void PuzzlePage::refreshNumPopulatedPuzzles(void)
{
	int numPuzzles = 0;
	for (int i = 0; i < kNumPuzzlesPerPage; ++i)
	{
		if (mPuzzles[i] && mPuzzles[i]->isPopulated())
			++numPuzzles;
	}
	setNumPuzzles(numPuzzles);
}

void PuzzlePage::positionHeaderLabel(CCLabelBMFont* headerLabel)
{
    if (mPuzzles[1])
    {
        CCNode* centerNode = mPuzzles[1]->getCenteredNode();
        if (centerNode)
        {
            LangFX::PuzzlePageSettings settings = PuzzlePage::getSettings();
            CCRect bounds = CMUtils::boundsInSpace(this, centerNode);
            
            CCPoint viewportTop = ccp(bounds.getMidX(), bounds.getMidY() + settings.puzzleBoardHeight / 2);
            viewportTop = convertToWorldSpace(viewportTop);
            viewportTop.y += (mScene->getViewportHeight() - viewportTop.y) / 2;
            viewportTop = convertToNodeSpace(viewportTop);
            
            CCPoint viewTop = ccp(bounds.getMidX(), bounds.getMidY() + settings.puzzleBoardHeight / 2);
            viewTop = convertToWorldSpace(viewTop);
            viewTop = mScene->getBaseSprite()->convertToNodeSpace(viewTop);
            viewTop.y += (mScene->getViewHeight() - viewTop.y) / 2;
            viewTop = mScene->getBaseSprite()->convertToWorldSpace(viewTop);
            viewTop = convertToNodeSpace(viewTop);
            
            headerLabel->setPosition(ccp(mScene->getViewWidth() / 2,
                                         MIN(viewportTop.y, viewTop.y) - settings.defaultOffsetY));
        }
    }
}

void PuzzlePage::setHeaderLabel(CCLabelBMFont* headerLabel)
{
	if (mLayers[kHeaderLayer])
	{
		mLayers[kHeaderLayer]->removeAllChildren();

		if (headerLabel)
        {
			mLayers[kHeaderLayer]->addChild(headerLabel);
            positionHeaderLabel(headerLabel);
        }
        mHeaderLabel = headerLabel;
	}
}

void PuzzlePage::setPuzzleAtIndex(int index, Puzzle* puzzle)
{
	if (index < 0 || index >= kNumPuzzlesPerPage || mPuzzles[index] == NULL)
        return;

	mPuzzles[index]->getPuzzleBoard()->setData(puzzle);
	mPuzzles[index]->getPuzzleBoard()->enableMenuMode(true);
    
    if (puzzle == NULL)
        mPuzzles[index]->setString("");
    else
    {
        const char* puzzleName = puzzle->getName();
        if (puzzleName == NULL)
            mPuzzles[index]->setString("");
        else
        {
            std::string puzzleString(SceneController::localizeString(puzzleName));
            mPuzzles[index]->setString(CMUtils::strConcatVal("", index + 1).append(". ").append(puzzleString).c_str());
        }
    }
	
	mPuzzles[index]->setIsPopulated(puzzle != NULL);
	mPuzzles[index]->resizeBoard();

    refreshNumPopulatedPuzzles();
}

void PuzzlePage::setLockedAtIndex(int index, bool locked)
{
	if (index < 0 || index >= kNumPuzzlesPerPage || mPuzzles[index] == NULL)
        return;
	mPuzzles[index]->setIsLocked(locked);
}

void PuzzlePage::setSolvedAtIndex(int index, bool solved)
{
	if (index < 0 || index >= kNumPuzzlesPerPage || mPuzzles[index] == NULL)
        return;
	mPuzzles[index]->setIsSolved(solved);
}

void PuzzlePage::setHighlightColor(uint color)
{
	for (int i = 0; i < kNumPuzzlesPerPage; ++i)
	{
		if (mPuzzles[i])
			mPuzzles[i]->setHighlightColor(color);
	}
}

void PuzzlePage::highlightPuzzle(int index, bool enable)
{
	if (index < 0 || index >= kNumPuzzlesPerPage || mPuzzles[index] == NULL)
        return;

	for (int i = 0; i < kNumPuzzlesPerPage; ++i)
	{
		if (mPuzzles[i])
			mPuzzles[i]->setIsHighlighted(false);
	}

	if (enable && mPuzzles[index])
		mPuzzles[index]->setIsHighlighted(enable);

}

void PuzzlePage::clear()
{
	for (int i = 0; i < kNumPuzzlesPerPage; ++i)
	{
		if (mPuzzles[i])
			mPuzzles[i]->setIsPopulated(false);
	}

	refreshNumPopulatedPuzzles();
}
