#ifndef __ANIM_PLAYER_PIECE_H__
#define __ANIM_PLAYER_PIECE_H__

#include "cocos2d.h"
#include <Puzzle/View/Playfield/PlayerPiece.h>
class MovieClip;
class SpriteColorer;
class FloatTweener;
USING_NS_CC;

class AnimPlayerPiece : public PlayerPiece
{
public:
	AnimPlayerPiece(Player* player, bool shouldPlaySounds = true);
	virtual ~AnimPlayerPiece(void);

	virtual bool init(void);

    virtual void transitionOut(float duration);
	virtual void enableMenuMode(bool enable);
	virtual void setPositionAestheticOnly(const CCPoint& value);
    virtual void refreshAesthetics(void);
	virtual void onEvent(int evType, void* evData);
	virtual void syncWithPlayerPiece(PlayerPiece* playerPiece);
	virtual void syncWithData(void);
	virtual void moveTo(const Coord& pos);
    virtual void didFinishMoving(void);
	virtual void playerValueDidChange(uint code, int value);
	virtual void advanceTime(float dt);
	virtual void visit(void);

    
	bool getShouldPlaySounds(void) const { return mShouldPlaySounds; }
	void setShouldPlaySounds(bool value) { mShouldPlaySounds = value; }

protected:
    virtual const char* getIdleMidFramesPrefix() = 0;
    virtual const char* getMovingMidFramesPrefix() = 0;
	void setAllClipShaderPrograms(CCGLProgram* program);
	void transitionToSkinColor(const ccColor3B& color);
	void didTeleport();
    
    void syncStationaryClipFrames();
    void syncMovingClipFrames();

	virtual void setState(PlayerPiece::PPState value);
	void setSkinColor(const ccColor3B& value);
    
    virtual void reuse(void);
    virtual void hibernate(void);
    virtual int getReuseKey(void) = 0;

private:
	static const int kNumOrientations = 4;
	static const float kSingleMoveDuration;
    static const float kMirroredAlpha;
    static const float kColorTransitionDuration;
	static const float kTeleportTwinkleDuration;
	static int clipIndexForPlayerOrientation(uint orientation);

	int getCurrentClipIndex(void) const { return mCurrentClipIndex; }
	void setCurrentClipIndex(int value);
	MovieClip* getCurrentStationaryLowerClip(void) const { return mStationaryLowerClips[mCurrentClipIndex]; }
    MovieClip* getCurrentStationaryMidClip(void) const { return mStationaryMidClips[mCurrentClipIndex]; }
	MovieClip* getCurrentStationaryUpperClip(void) const { return mStationaryUpperClips[mCurrentClipIndex]; }
	MovieClip* getCurrentMovingLowerClip(void) const { return mMovingLowerClips[mCurrentClipIndex]; }
    MovieClip* getCurrentMovingMidClip(void) const { return mMovingMidClips[mCurrentClipIndex]; }
	MovieClip* getCurrentMovingUpperClip(void) const { return mMovingUpperClips[mCurrentClipIndex]; }
	float getTweenMoveTo(void); // Not const because uses cocos2dx non-const method
	void setTweenMoveTo(float value);
    void clampToTileGrid(void);

	bool mIsInitializing;
    bool mDidTeleportThisFrame;
    bool mShouldPlaySounds;
    int mCurrentClipIndex;
    float mMagicX;
	float mMirrorX;
    MovieClip* mStationaryLowerClips[kNumOrientations];
    MovieClip* mStationaryMidClips[kNumOrientations];
	MovieClip* mStationaryUpperClips[kNumOrientations];
    MovieClip* mMovingLowerClips[kNumOrientations];
    MovieClip* mMovingMidClips[kNumOrientations];
	MovieClip* mMovingUpperClips[kNumOrientations];
    CCSprite* mSkinColoree;
    SpriteColorer* mSkinColorer;
    FloatTweener* mMoveTweener;
	CCGLProgram* mNormalShader;
	CCGLProgram* mColorMagicShader;
	CCGLProgram* mMirrorImageShader;
    CCArray* mTwinkles;
};
#endif // __ANIM_PLAYER_PIECE_H__
