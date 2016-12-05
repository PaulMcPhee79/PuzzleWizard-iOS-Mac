#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "cocos2d.h"
#include <Interfaces/IPlayerView.h>
#include <Utils/CMTypes.h>
#include <climits>
USING_NS_CC;

class Player : public CCObject
{
public:
	enum PlayerType { INVALID_PLAYER = 0, HUMAN_PLAYER, MIRRORED_MATE };

	// Orientations
	enum {
		kNorthernOrientation = 0,
		kEasternOrientation = 1,
		kSouthernOrientation = 2,
		kWesternOrientation = 3
	};


	// Property change codes
    static const uint kValueProperty = 1;
    static const uint kValueColorMagic = 2;
    static const uint kValueMirroredMate = 3;
	static const uint kValueColor = 4;
	static const uint kValueOrientation = 5;
	static const uint kValuePosition = 6;
	static const uint kValueTeleported = 7;
	static const uint kValueAll = LONG_MAX;

	// Property change constants
    static int maxColorMagicMoves(void);

	Player(uint colorKey, const Coord& position = CMCoordZero, uint orientation = kSouthernOrientation);
	virtual ~Player(void);

	static uint getMirroredOrientation(uint orientation);
	static Coord orientation2Coord(uint orientation);
	static uint coord2Orientation(const Coord& moveVec);

	void registerView(IPlayerView* view);
	void deregisterView(IPlayerView* view);
	virtual void broadcastProperties(void) { }
	void rotateCW(int increments);
	void rotateACW(int increments);
	virtual void beginMoveTo(const Coord& pos);
	void didBeginMoving(void);
	virtual void didFinishMoving(void);
    virtual void idle(void);
    virtual void treadmill(const Coord& dir);
	void silentMoveTo(const Coord& pos);
	virtual void moveTo(const Coord& pos);
	virtual void teleportTo(const Coord& pos);
	virtual void reset(void);
	virtual Player* clone(void) const = 0;
	virtual Player* devClone(void) const = 0;

	virtual const char* getCostumeName() const { return "player"; }
	int getID(void) const { return mID; }
	virtual PlayerType getType(void) { return INVALID_PLAYER; }
	uint getOrientation(void) const { return mOrientation; }
	void setOrientation(uint value)
	{
		setPrevOrientation(mOrientation);
		mOrientation = value;
		notifyPropertyChange(kValueProperty, -1);
        
        if (mOrientation != getPrevOrientation())
            notifyPropertyChange(kValueOrientation, -1);
	}
	uint getPrevOrientation(void) const { return mPrevOrientation; }
	uint getColorKey(void) const { return mColorKey; }
	void setColorKey(uint value)
	{
		setPrevColorKey(mColorKey);
		mColorKey = value;
		notifyPropertyChange(kValueProperty, -1);
	}
	uint getPrevColorKey(void) const { return mPrevColorKey; }
	uint getFutureColorKey(void) const { return mFutureColorKey; }
	void setFutureColorKey(uint value) { mFutureColorKey = value; }
	Coord getPosition(void) const { return mPosition; }
	void setPosition(const Coord& value)
	{
		mPosition = value;
		notifyPropertyChange(kValueProperty, -1);
	}
	Coord getViewPosition(void) const
	{
		return cmc(mPosition.x, mViewOffset.y - mPosition.y);
	}
	Coord getViewOffset(void) { return mViewOffset; }
	void setViewOffset(const Coord& offset) { mViewOffset = offset; }
	Coord getQueuedMove(void) const { return mQueuedMove; }
	void setQueuedMove(const Coord& value) { mQueuedMove = value; }
	vec2 getMoveDimensions(void) const { return mMoveDimensions; }
	void setMoveDimensions(vec2 value)
	{
		mMoveDimensions = value;
		notifyPropertyChange(kValueProperty, -1);
	}
	bool isMoving(void) const { return mIsMoving; }
	bool getDidMove(void) const { return mDidMove; }
	void setDidMove(bool value) { mDidMove = value; }
	Coord getIsMovingTo(void) const { return mIsMovingTo; }
	bool isColorMagicActive(void) const { return mNumColorMagicMoves > 0; }
	void setIsColorMagicActive(bool value) { setNumColorMagicMoves(value ? Player::maxColorMagicMoves() : 0); }
	uint getFunction(void) const { return mFunction; }
	void setFunction(uint value) { mFunction = value; }
	int getFunctionData(void) const { return mFunctionData; }
	void setFunctionData(int value) { mFunctionData = value; }
	uint getDevOrientation(void) const { return mDevOrientation; }
	void setDevOrientation(uint value)
	{
		mDevOrientation = value;
        setOrientation(value);
        setPrevOrientation(value);
	}
	uint getDevColorKey(void) const { return mDevColorKey; }
	void setDevColorKey(uint value)
	{
		mDevColorKey = value;
        setColorKey(value);
        setPrevColorKey(value);
	}
	Coord getDevPosition(void) const { return mDevPosition; }
	void setDevPosition(const Coord& value)
	{
		mDevPosition = value;
		setPosition(value);
	}
	int getNumColorMagicMoves() const { return mNumColorMagicMoves; }
    void setNumColorMagicMoves(int value)
	{
		bool wasActive = isColorMagicActive();
        mNumColorMagicMoves = MAX(0, MIN(Player::maxColorMagicMoves(), value));
        notifyPropertyChange(kValueColorMagic, mNumColorMagicMoves);
        
        if (wasActive != isColorMagicActive())
            notifyPropertyChange(kValueProperty, -1);
	}
	
protected:
	void notifyPropertyChange(uint code, int value);

private:
	Player(const Player& other);
	Player& operator=(const Player& rhs);

	void notifyWillBeginMoving(void);
    void notifyDidFinishMoving(void);
    void notifyDidIdle(void);
    void notifyDidTreadmill(void);
	void setPrevOrientation(uint value) { mPrevOrientation = value; }
	void setPrevColorKey(uint value) { mPrevColorKey = value; }
	void setIsMoving(bool value) { mIsMoving = value; }
	void setIsMovingTo(const Coord& value) { mIsMovingTo = value; }

    static int kMaxColorMagicMoves;
    
	int mID;
	uint mOrientation;
    uint mColorKey;
    uint mPrevOrientation;
    uint mPrevColorKey;
    uint mFutureColorKey;
    Coord mPosition;
    Coord mQueuedMove;
    vec2 mMoveDimensions;

    bool mIsMoving;
    bool mDidMove;
    Coord mIsMovingTo;
	Coord mViewOffset;

    int mNumColorMagicMoves;

    uint mFunction;
    int mFunctionData;

    uint mDevOrientation;
    uint mDevColorKey;
    Coord mDevPosition;

	bool mViewsLocked;
	std::set<IPlayerView*>* mViews;
};
#endif // __PLAYER_H__
