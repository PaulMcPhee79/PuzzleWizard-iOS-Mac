#ifndef __JUKEBOX_H__
#define __JUKEBOX_H__

#include "cocos2d.h"
#include <Events/EventDispatcher.h>
class IEventListener;
USING_NS_CC;

class Jukebox
{
public:
    enum JukeboxType { JB_EVENT_BASED, JB_MANUAL_TIMER }; // TODO: make subclasses - this makes the whole class messy
	static int EV_TYPE_JUKEBOX_SONG_ENDED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	Jukebox(JukeboxType type, std::map<std::string, double>* songs, IEventListener* listener = NULL);
	~Jukebox(void);

	bool isPaused(void) const { return mIsPaused || getSongIndex() == -1; }
    bool isWaiting(void) const { return mIsWaiting; }
    bool isShuffleEnabled(void) const { return mShuffle; }
    void enableShuffle(bool value) { mShuffle = value; }
    std::string getCurrentSongName(void) const;
    std::string getPreviousSongName(void) const;

    void pause(void);
    void resume(void);
    void prevSong(void);
    void nextSong(void);
    void addSong(std::string name, double duration);
    void removeSong(std::string name);
    void clearSongs(void);
    void randomize(void);
    void advanceTime(float dt);
    
private:
    Jukebox();
    
    int getNumSongs(void) const { return (int)mSongNames.size(); }
    int getSongIndex(void) const { return mSongIndex; }
    void setSongIndex(int value)
    {
        mPrevSongIndex = mSongIndex;
        mSongIndex = value;
    }
    void enqueueNextSong(void);
    
    JukeboxType mType;
	bool mIsPaused;
    bool mIsWaiting;
    bool mShuffle;
    int mSongIndex;
    int mPrevSongIndex;
    double mSongDuration;
    double mSongTime;
    double mTotalTime;
    std::vector<std::string> mSongNames;
    std::map<std::string, double> mSongDurations;
    IEventListener* mListener;
};
#endif // __JUKEBOX_H__
