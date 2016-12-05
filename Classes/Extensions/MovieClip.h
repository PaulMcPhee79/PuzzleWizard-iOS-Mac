#ifndef __MOVIE_CLIP_H__
#define __MOVIE_CLIP_H__

#include "cocos2d.h"
#include <Events/EventDispatcher.h>
#include <Interfaces/IAnimatable.h>
USING_NS_CC;

class MovieClip : public CCSprite, public EventDispatcher, public IAnimatable
{
public:
	static int EV_TYPE_MOVIE_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	MovieClip(void);
	virtual ~MovieClip(void);

	static MovieClip* createWithFrames(CCArray* spriteFrames, float fps = 12.0f, bool autorelease = true);
	bool initWithFrames(CCArray* spriteFrames, float fps = 12.0f);

	virtual void advanceTime(float dt);

	int addFrame(CCSpriteFrame* frame);
	int addFrame(CCSpriteFrame* frame, float duration);
	void insertFrameAtIndex(CCSpriteFrame* frame, int index);
	void removeFrameAtIndex(int index);
	CCSpriteFrame* getFrameAtIndex(int index);
	void setFrameAtIndex(CCSpriteFrame* frame, int index);

	void play(void);
	void pause(void);

	virtual bool isComplete(void) const { return false; }
	virtual void* getTarget(void) const { return NULL; }
	float getDuration(void) { return mTotalDuration; }
	bool getLoop(void) const { return mLoop; }
	void setLoop(bool value) { mLoop = value; }
	bool isPlaying(void) const { return mIsPlaying; }
	int getNumFrames(void) const { return mFrames ? mFrames->count() : 0; }
	int getCurrentFrame(void) const { return mCurrentFrame; }
	void setCurrentFrame(int value);
	float getFps(void) const { return 1.0f / mDefaultFrameDuration; }
	void setFps(float value);
	float getDurationAtIndex(int index) const;
	void setDurationAtIndex(float value, int index);

private:
	void updateCurrentFrame(void);
	bool isValidIndex(int index) const;

	std::vector<float> mFrameDurations;
	float mDefaultFrameDuration;
	float mTotalDuration;
	float mCurrentTime;
	bool mLoop;
	bool mIsPlaying;
	int mCurrentFrame;
	CCArray *mFrames;
};
#endif // __MOVIE_CLIP_H__
