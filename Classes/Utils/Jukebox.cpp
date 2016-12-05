
#include "Jukebox.h"
#include "Interfaces/IEventListener.h"
#include <Utils/BridgingUtility.h>
#include "Utils/Utils.h"
#include <algorithm>
USING_NS_CC;

Jukebox::Jukebox(JukeboxType type, std::map<std::string, double>* songs, IEventListener* listener)
    :
    mType(type),
    mIsPaused(true),
    mIsWaiting(false),
    mShuffle(false),
    mSongIndex(-1),
    mPrevSongIndex(-1),
    mSongDuration(0),
    mSongTime(0),
    mTotalTime(0),
    mListener(listener)
{
	if (songs && songs->size() > 0)
    {
        mSongIndex = 0;
        for (std::map<std::string, double>::iterator it = songs->begin(); it != songs->end(); ++it)
        {
            mSongNames.push_back(it->first);
            mSongDurations[it->first] = it->second;
        }
    }
}

Jukebox::~Jukebox(void)
{
	mListener = NULL;
}

std::string Jukebox::getCurrentSongName(void) const
{
    int songIndex = getSongIndex();
    if (songIndex >= 0 && songIndex < getNumSongs())
        return mSongNames[songIndex];
    else
        return "";
}

std::string Jukebox::getPreviousSongName(void) const
{
    int songIndex = mPrevSongIndex;
    if (songIndex >= 0 && songIndex < getNumSongs())
        return mSongNames[songIndex];
    else
        return "";
}

void Jukebox::pause(void)
{
    mIsPaused = true;
}

void Jukebox::resume(void)
{
    mIsPaused = false;
}

void Jukebox::prevSong(void)
{
    if (!isShuffleEnabled())
    {
        for (int i = 0, numSongs = getNumSongs()-1; i < numSongs; ++i)
            enqueueNextSong();
    }
}

void Jukebox::nextSong(void)
{
    if (mIsWaiting)
        mIsWaiting = false;
    else
        enqueueNextSong();
}

void Jukebox::addSong(std::string name, double duration)
{
    std::vector<std::string>::iterator findIt = std::find(mSongNames.begin(), mSongNames.end(), name);
    if (findIt == mSongNames.end())
    {
        mSongNames.push_back(name);
        mSongDurations[name] = duration;
    }
}

void Jukebox::removeSong(std::string name)
{
    std::vector<std::string>::iterator findIt = std::find(mSongNames.begin(), mSongNames.end(), name);
    if (findIt != mSongNames.end())
    {
        mSongNames.erase(std::remove(mSongNames.begin(), mSongNames.end(), name));
        mSongDurations.erase(mSongDurations.find(name));
    }
}

void Jukebox::clearSongs(void)
{
    mSongNames.clear();
    mSongDurations.clear();
    setSongIndex(-1);
    mPrevSongIndex = -1;
}

void Jukebox::randomize(void)
{
    int numSongs = getNumSongs();
    for (int i = 0; i < numSongs; ++i)
    {
        int rndIndex = CMUtils::nextRandom(0, numSongs - (i + 1));
        if (rndIndex < 0 || rndIndex >= numSongs)
            continue;
        
        std::string name = mSongNames[rndIndex];
        mSongNames.erase(std::remove(mSongNames.begin(), mSongNames.end(), name));
        mSongNames.push_back(name);
        //CCLog(name.c_str());
    }
}

void Jukebox::enqueueNextSong(void)
{
    int numSongs = getNumSongs();
    if (numSongs == 0)
        setSongIndex(-1);
    else if (isShuffleEnabled() && numSongs > 3)
    {
        int songIndex = getSongIndex();
        while (songIndex == getSongIndex() || songIndex == mPrevSongIndex)
            songIndex = CMUtils::nextRandom(0, numSongs-1);
        setSongIndex(songIndex);
    }
    else
    {
        if (getSongIndex() >= numSongs - 1)
            setSongIndex(0);
        else
            setSongIndex(getSongIndex() + 1);
    }
    
    mSongTime = 0;
    mSongDuration = getSongIndex() != -1 ? mSongDurations[mSongNames[getSongIndex()]] : 0;
    mIsWaiting = true;
    if (mListener)
        mListener->onEvent(EV_TYPE_JUKEBOX_SONG_ENDED(), this);
}

void Jukebox::advanceTime(float dt)
{
    if (mSongDuration == 0)
        enqueueNextSong();
    else
    {
        switch (mType)
        {
            case JB_EVENT_BASED:
                if (BridgingUtility::didBackGroundAudioComplete())
                    enqueueNextSong();
                break;
            case JB_MANUAL_TIMER:
            {
                if (isPaused() || isWaiting())
                    break;;
                
                mSongTime += dt;
                mTotalTime += dt;
                if (mSongTime >= mSongDuration)
                    enqueueNextSong();
            }
                break;
        }
    }
}
