
#include "MovieClip.h"
#include <climits>
USING_NS_CC;

MovieClip::MovieClip(void)
:
mDefaultFrameDuration(0),
mTotalDuration(0),
mCurrentTime(0),
mLoop(true),
mIsPlaying(false),
mCurrentFrame(0),
mFrames(NULL)
{

}

MovieClip::~MovieClip(void)
{
	CC_SAFE_RELEASE_NULL(mFrames);
}

MovieClip* MovieClip::createWithFrames(CCArray* spriteFrames, float fps, bool autorelease)
{
	MovieClip *clip = new MovieClip();
	if (clip && clip->initWithFrames(spriteFrames, fps))
    {
		if (autorelease)
			clip->autorelease();
        return clip;
    }
    CC_SAFE_DELETE(clip);
    return NULL;
}

bool MovieClip::initWithFrames(CCArray* spriteFrames, float fps)
{
	if (!this->init())
		return false;

	CCAssert(spriteFrames && spriteFrames->count() > 0, "MovieClip requires non-NULL spriteFrames array with length > 0.");
	mDefaultFrameDuration = (fps == 0.0f) ? LONG_MAX : 1.0 / fps;
	mLoop = true;
    mIsPlaying = true;
    mTotalDuration = 0.0;
    mCurrentTime = 0.0;
    mCurrentFrame = 0;
	mFrames = new CCArray();
	mFrames->initWithCapacity(spriteFrames->count());
	
	CCObject* obj;
	CCARRAY_FOREACH(spriteFrames, obj)
    {
        CCSpriteFrame* frame = (CCSpriteFrame*)obj;
        if (frame)
			addFrame(frame);
    }

	setFps(fps);
	updateCurrentFrame();

	return true;
}

int MovieClip::addFrame(CCSpriteFrame* frame)
{
	return addFrame(frame, mDefaultFrameDuration);
}

int MovieClip::addFrame(CCSpriteFrame* frame, float duration)
{
	if (frame && duration >= 0)
	{
		mTotalDuration += duration;
		mFrames->addObject(frame);
		mFrameDurations.push_back(duration);
	}
	return mFrames->count() - 1;
}

void MovieClip::insertFrameAtIndex(CCSpriteFrame* frame, int index)
{
	if (frame && isValidIndex(index))
	{
		mFrames->insertObject(frame, index);
		mFrameDurations.insert(mFrameDurations.begin(), index);
		mTotalDuration += mDefaultFrameDuration;
	}
}

void MovieClip::removeFrameAtIndex(int index)
{
	if (isValidIndex(index))
	{
		mTotalDuration -= getDurationAtIndex(index);
		mFrames->removeObjectAtIndex(index);
		mFrameDurations.erase(mFrameDurations.begin()+index);
	}
}

CCSpriteFrame* MovieClip::getFrameAtIndex(int index)
{
	if (isValidIndex(index))
		return (CCSpriteFrame*)mFrames->objectAtIndex(index);
	else
		return NULL;
}

void MovieClip::setFrameAtIndex(CCSpriteFrame* frame, int index)
{
	if (isValidIndex(index))
		mFrames->replaceObjectAtIndex(index, frame);
}

void MovieClip::setCurrentFrame(int value)
{
	if (isValidIndex(value))
	{
		mCurrentFrame = value;
		mCurrentTime = 0.0f;

		for (int i = 0; i < value; ++i)
			mCurrentTime += mFrameDurations[i];

		updateCurrentFrame();
	}
}

void MovieClip::setFps(float value)
{
	float newFrameDuration = value == 0.0f ? LONG_MAX : 1.0f / value;
    float acceleration = newFrameDuration / mDefaultFrameDuration;
    mCurrentTime *= acceleration;
    mDefaultFrameDuration = newFrameDuration;

	int numFrames = getNumFrames();
    for (int i = 0; i < numFrames; ++i)
        setDurationAtIndex(getDurationAtIndex(i) * acceleration, i);
}

float MovieClip::getDurationAtIndex(int index) const
{
	if (isValidIndex(index))
		return mFrameDurations[index];
	else
		return 0.0f;
}

void MovieClip::setDurationAtIndex(float value, int index)
{
	if (value >= 0 && isValidIndex(index)) {
        float oldValue = mFrameDurations[index];
		mFrameDurations[index] = value;
        mTotalDuration += value - oldValue;
    }
}

void MovieClip::play(void)
{
	mIsPlaying = true;
}

void MovieClip::pause(void)
{
	mIsPlaying = false;
}

void MovieClip::updateCurrentFrame(void)
{
	if (mFrames)
		this->setDisplayFrame((CCSpriteFrame*)mFrames->objectAtIndex((unsigned int)mCurrentFrame));
}

bool MovieClip::isValidIndex(int index) const
{
	return mFrames && index >= 0 && index < (int)mFrames->count();
}

void MovieClip::advanceTime(float dt)
{
	if (mLoop && mCurrentTime == mTotalDuration) mCurrentTime = 0.0f;
    if (!mIsPlaying || dt == 0.0f || mCurrentTime == mTotalDuration) return;

    float durationSum = 0.0f;
    float previousTime = mCurrentTime;
    float restTime = mTotalDuration - mCurrentTime;
    float carryOverTime = dt > restTime ? dt - restTime : 0.0f;
    mCurrentTime = MIN(mTotalDuration, mCurrentTime + dt);

	for(std::vector<int>::size_type i = 0; i != mFrameDurations.size(); i++)
    {
		float frameDuration = mFrameDurations[i];
        if (durationSum + frameDuration >= mCurrentTime)
        {
            if (mCurrentFrame != i)
            {
                mCurrentFrame = (int)i;
                updateCurrentFrame();
                //PlayCurrentSound();
            }
            break;
        }

        durationSum += frameDuration;
    }

	if (previousTime < mTotalDuration && mCurrentTime == mTotalDuration && this->hasEventListenerForType(EV_TYPE_MOVIE_COMPLETED()))
		dispatchEvent(EV_TYPE_MOVIE_COMPLETED(), this);

    this->advanceTime(carryOverTime);
}
