#ifndef __PUZZLE_BOARD_H__
#define __PUZZLE_BOARD_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Puzzle/Data/Puzzle.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/IPuzzleView.h>
#include <Interfaces/ITileDecorator.h>
#include <Interfaces/IResDependent.h>
#include <Utils/CMTypes.h>
#include <Utils/Transitions.h>
class ShieldManager;
class TilePiece;
class PlayerPiece;
class Player;
class ByteTweener;
class FloatTweener;
class TileConveyorBelt;
class TileRotator;
class TileSwapper;
class MovieClip;
USING_NS_CC;

class PuzzleBoard : public Prop, public EventDispatcher, public IEventListener, public IPuzzleView, public ITileDecorator, public IResDependent, public ILocalizable
{
public:
	static int EV_TYPE_DID_TRANSITION_IN() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_DID_TRANSITION_OUT() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_RESET_REQUESTED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    static int EV_TYPE_BG_EXPOSURE_WILL_BEGIN() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    static int EV_TYPE_BG_EXPOSURE_DID_END() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
	PuzzleBoard(int category);
	virtual ~PuzzleBoard(void);

	static PuzzleBoard* create(int category, Puzzle* puzzle, bool autorelease = true);
	bool initWithPuzzle(Puzzle* puzzle);

    virtual void resolutionDidChange(void);
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
	void transitionIn(float duration, float columnDelay, float rowDelay);
	void transitionOut(float duration, float columnDelay, float rowDelay);
	void enableMenuMode(bool enable, bool useMenuScale = true);
	void setData(Puzzle* puzzle);
	void syncWithData();
	void enableHighlight(bool enable);
    void updateBounds(const CCPoint& pos);
    void boundsDidChange(void) { }
    void refreshColorScheme(void);
	void enablePath(Player* player, const Coord* path, int numSteps);
	void disablePath(void);
    void displayPathNotFound(int tileIndex);
    void hidePathNotFound(void);
	void addPlayerPiece(Player* player);
	void removePlayerPiece(Player* player, float transitionOutDuration = 1.0f);
	virtual void onEvent(int evType, void* evData);
	void reset(bool playSound = false);
	virtual void advanceTime(float dt);
	void postAdvanceTime(float dt);

	virtual GLubyte decoratorValueForKey(uint key);
	virtual void decorationDidChange(TilePiece* tilePiece);

	virtual void puzzleSoundShouldPlay(const char* soundName);
	virtual void puzzlePlayerWillMove(Player* player);
	virtual void puzzlePlayerDidMove(Player* player);
    virtual void puzzleShieldDidDeploy(int tileIndex);
    virtual void puzzleShieldWasWithdrawn(int tileIndex);
    virtual void puzzleTilesShouldRotate(const int2d& tileIndexes);
    virtual void puzzleTileSwapWillBegin(const int2d& swapIndexes, bool isCenterValid);
	virtual void puzzleConveyorBeltWillMove(Coord moveDir, int wrapIndex, const int1d& tileIndexes, int numTiles);
    virtual void puzzleWasSolved(int tileIndex);

	const char* getName(void) const { return mPuzzle ? mPuzzle->getName() : NULL; }
	bool isTransitioning(void) const { return mIsTransitioning; }
	vec2 getTileDimensions(void) const;
	vec2 getBoardDimensions(void) const;
	vec2 getScaledTileDimensions(void);
	vec2 getScaledBoardDimensions(void);
	CCRect getInvertedBoardBounds(void); // Can't be const because of cocos2dx usage.
    CCRect getGlobalBoardClippingBounds(void); // Can't be const because of cocos2dx usage.
    CCRect getIqTagGlobalBounds(void);
	void setHighlightColor(const ccColor3B& value);
	TilePiece* getTilePieceAtIndex(int index) const;

protected:
	void setPuzzle(Puzzle* value);
	void refreshPlayerZValues(void);
	void refreshTeleportOpacity(GLubyte value);

private:
	static const int kLowerCanvas = 0;
	static const int kLowerPlayerCanvas = 1;
    static const int kMidCanvas = 2;
	static const int kUpperPlayerCanvas = 3;
    static const int kUpperCanvas = 4;
    static const int kMaxCanvas = kUpperCanvas;
	static const int kCanvasLen = kMaxCanvas + 1;
	
    int getEdgeStatus(int tileIndex);
    void setIQLabelStr(std::string str);
    void updateIQText(const Puzzle* puzzle);
	void setCanvasScale(float value);
	void setTransitioning(bool value);
    void transitionIqTag(bool inwards, float duration, float delay);
	void transition(int tag, float duration, float columnDelay, float rowDelay, TRANS_FUNC transition);
	float calculateCanvasScaler(void) const;
	void clearPlayerPieces(float transitionOutDuration = 1.0f);

    bool isBatchingEnabled(void) const { return mIsBatchingEnabled; }
    void enableBatching(bool value) { mIsBatchingEnabled = value; }
	void batchTilePiece(TilePiece* tilePiece);
	void rebatchTilePieces(void);
	void unbatchAllTilePieces(void);
	void tilePieceKeyStateDidChange(TilePiece* tilePiece);

	PlayerPiece* addPlayerToEffect(Player* player);
	void tileConveyorBeltCompleted(TileConveyorBelt* conveyorBelt);
	void tileRotatorCompleted(TileRotator* rotator);
	void tileSwapperCompleted(TileSwapper* swapper);
    void forceCompleteActiveAnimations(void);

	bool mIsLocked;
    bool mIsBatchingEnabled;
    bool mIsTransitioning;
    bool mIsMenuModeEnabled;
	float mLastPlayerTransitionDelay; // Used in SetData hack.
    Puzzle* mPuzzle;
	std::vector<TilePiece *> mTilePieces;
	CCDictionary* mPlayerPieces; // <int (Player->getID(), PlayerPiece>
    CCArray* mAddQueue;
    CCArray* mRemoveQueue;
    
    std::vector<TileConveyorBelt*> mAnimatingConveyorBelts;
    std::vector<TileRotator*> mAnimatingRotators;
    std::vector<TileSwapper*> mAnimatingTileSwappers;

	int mNumPathIndexes;
	int* mPathIndexes;
	ByteTweener* mPathTweener;
    
    float mPathNotFoundTimer;
    CCSprite* mPathNotFound;

	ByteTweener* mTeleportTweener;
	ByteTweener* mKeyTweener;
    MovieClip* mColorArrowClip;

	std::vector<FloatTweener*> mTransitions;

    CCLabelBMFont* mIqLabel[2];
    CCSprite* mIqTag;
    CCNode* mIqTagNode;
    ByteTweener* mIqTweener;
    
    CCSprite* mHighlighter;
	CCSpriteBatchNode* mPreBatch;
	CCSpriteBatchNode* mPostBatch;
	CCNode* mKeyNode;
    CCNode* mCanvas[kCanvasLen];

    ShieldManager* mShieldManager;
};
#endif // __PUZZLE_BOARD_H__
