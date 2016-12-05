
#include "GameProgress.h"
#include <Managers/FileManager.h>
#include <Utils/Utils.h>
#include <Utils/UtilsNS.h>
#include <stdint.h>
USING_NS_CC;

typedef struct SaveData
{
    explicit SaveData(const u8* data, int32_t dataVersion, int32_t minsPlayed, int32_t numLevels, int32_t numPuzzlesPerLevel)
        : _data(NULL), _dataVersion(dataVersion), _minsPlayed(minsPlayed), _numLevels(numLevels), _numPuzzlesPerLevel(numPuzzlesPerLevel)
    {
        size_t size = _numLevels * _numPuzzlesPerLevel * sizeof(u8);
        _data = (u8*)malloc(size);
        if (_data)
        {
            memset(_data, 0, size);
            memcpy(_data, data, size);
        }
    }
    ~SaveData(void)
    {
       free(_data), _data = 0;
    }
    u8* _data;
    int32_t _dataVersion;
    int32_t _minsPlayed;
    int32_t _numLevels;
    int32_t _numPuzzlesPerLevel;
} SaveData;

static const int kDataVersion = 4; // 1 , 2:added minsPlayed(u8) , 3:refactored minsPlayed(int), 4:added <stdint.h>
static const u8 kPlayedBit = 1 << 0;
static const u8 kSolvedBit = 1 << 1;

static const char* kFilePath = "PW_GameProgress";
static const char* kFileExt = ".dat";
const char* GameProgress::kFilePathExt = "PW_GameProgress.dat";

void loadCallback(void* arg);
static void saveCallback(void* arg);

GameProgress::GameProgress(int numLevels, int numPuzzlesPerLevel)
	:
mNumLevels(numLevels),
mNumPuzzlesPerLevel(numPuzzlesPerLevel),
mNumSolvedLevelsCache(-1),
mNumSolvedPuzzlesCache(-1),
mMinsPlayed(0),
mLevelData(0),
mExportData(0)
{
    prepareLevelDataForNumLevels(numLevels);
}

GameProgress::~GameProgress(void)
{
    free(mLevelData), mLevelData = 0;
    free(mExportData), mExportData = 0;
}

void GameProgress::prepareLevelDataForNumLevels(int numLevels)
{
    free(mLevelData);
    size_t size = numLevels * getNumPuzzlesPerLevel() * sizeof(u8);
    CCAssert(size > 0, "GameProgress::prepareLevelDataForNumLevels - 0 byte malloc detected.");
    mLevelData = (u8*)malloc(size);
    memset(mLevelData, 0, size);
    
    free(mExportData), mExportData = 0;
}

u8 GameProgress::valueForPuzzleIndex(int index) const
{
    return mLevelData[index];
}

int GameProgress::resolvedIndex(int levelIndex, int puzzleIndex) const
{
    return levelIndex * getNumPuzzlesPerLevel() + puzzleIndex;
}

int GameProgress::getNumSolvedLevels(void)
{
	if (mNumSolvedLevelsCache != -1)
        return mNumSolvedLevelsCache;

    mNumSolvedLevelsCache = 0;
    for (int i = 0; i < getNumLevels(); ++i)
    {
		if (getNumSolvedPuzzlesForLevel(i) == getNumPuzzlesPerLevel())
            ++mNumSolvedLevelsCache;
    }

    return mNumSolvedLevelsCache;
}

int GameProgress::getNumPuzzles(void) const
{
    return getNumLevels() * getNumPuzzlesPerLevel();
}

int GameProgress::getNumSolvedPuzzles(void)
{
	if (mNumSolvedPuzzlesCache != -1)
        return mNumSolvedPuzzlesCache;

    mNumSolvedPuzzlesCache = 0;
	for (int i = 0; i < getNumLevels(); ++i)
	{
		for (int j = 0; j < getNumPuzzlesPerLevel(); ++j)
		{
			if (hasSolved(i, j))
				++mNumSolvedPuzzlesCache;
		}
	}

    return mNumSolvedPuzzlesCache;
}

void GameProgress::setMinsPlayed(int value)
{
    mMinsPlayed = MIN(INT32_MAX, MAX(0, value));
}

void GameProgress::invalidateCaches(void)
{
	mNumSolvedPuzzlesCache = mNumSolvedLevelsCache = -1;
}

bool GameProgress::isValidIndexes(int levelIndex, int puzzleIndex) const
{
	return !(levelIndex < 0 || levelIndex >= getNumLevels() || puzzleIndex < 0 || puzzleIndex >= getNumPuzzlesPerLevel());
}

bool GameProgress::hasPlayed(int levelIndex, int puzzleIndex)
{
	if (!isValidIndexes(levelIndex, puzzleIndex))
        return false;
    return (valueForPuzzleIndex(resolvedIndex(levelIndex, puzzleIndex)) & kPlayedBit) == kPlayedBit;
}

void GameProgress::setPlayed(bool played, int levelIndex, int puzzleIndex)
{
	if (!isValidIndexes(levelIndex, puzzleIndex))
        return;

    if (played)
        mLevelData[resolvedIndex(levelIndex, puzzleIndex)] |= kPlayedBit;
    else
        mLevelData[resolvedIndex(levelIndex, puzzleIndex)] &= ~kPlayedBit;
}

bool GameProgress::hasSolved(int levelIndex, int puzzleIndex) const
{
	if (!isValidIndexes(levelIndex, puzzleIndex))
        return false;
    return (valueForPuzzleIndex(resolvedIndex(levelIndex, puzzleIndex)) & kSolvedBit) == kSolvedBit;
}

void GameProgress::setSolved(bool solved, int levelIndex, int puzzleIndex)
{
	if (!isValidIndexes(levelIndex, puzzleIndex))
        return;

    if (solved)
        mLevelData[resolvedIndex(levelIndex, puzzleIndex)] |= kSolvedBit;
    else
        mLevelData[resolvedIndex(levelIndex, puzzleIndex)] &= ~kSolvedBit;

	invalidateCaches();
}

int GameProgress::getNumSolvedPuzzlesForLevel(int levelIndex) const
{
	if (levelIndex < 0 || levelIndex >= getNumLevels())
        return 0;

    int numSolvedPuzzles = 0;
	for (int i = 0; i < getNumPuzzlesPerLevel(); ++i)
    {
        if (hasSolved(levelIndex, i))
            ++numSolvedPuzzles;
    }

    return numSolvedPuzzles;
}

UpgradeStatus GameProgress::syncWithData(const u8* data, ulong size, ulong& offset, bool merge)
{
    CCAssert(size > offset, "GameProgress::syncWithData - bad args.");
    if (offset > size)
        return US_NO_CHANGE;
    
    UpgradeStatus upgradeStatus = US_NO_CHANGE;
    
    int dataVersion = -1;
    if ((size - offset) >= sizeof(int32_t))
	{
		dataVersion = *(int32_t*)(data + offset);
		offset += sizeof(int32_t);
	}
    
    CCLog("GameProgress::DataVersion Done");
    
    // Ignore early non-release versions that are lying around in the cloud or on disk
    if (dataVersion < 4)
        return US_NO_CHANGE;

    if ((size - offset) >= sizeof(int32_t))
    {
    	mMinsPlayed = *(int32_t*)(data + offset);
        offset += sizeof(int32_t);
    }

    CCLog("GameProgress::MinsPlayed Done");

    int numLevels = 0;
	if ((size - offset) >= sizeof(int32_t))
	{
		numLevels = MIN(getNumLevels(), *(int32_t*)(data + offset));
		offset += sizeof(int32_t);
	}

	CCLog("GameProgress::NumLevels Done");
    
	// Make room for our levels.
    int numPuzzlesPerLevel = getNumPuzzlesPerLevel();
    if (getNumPuzzles() < numLevels * numPuzzlesPerLevel)
        prepareLevelDataForNumLevels(numLevels);
    
	int levelIndex = 0;
	while (levelIndex < numLevels)
	{
		for (int i = 0; i < numPuzzlesPerLevel && (offset + sizeof(u8)) <= size; ++i)
		{
			u8 token = *(u8*)(data + offset);
			u8* puzzleData = mLevelData + (levelIndex * numPuzzlesPerLevel + i);
            
            // Linked to GameProgressController's UpgradeStatus enum.
            if (((*puzzleData) | token) != *puzzleData)
                upgradeStatus = upgradeStatus == US_NO_CHANGE ? US_LOCAL_UPGRADE : US_FULL_UPGRADE;
            if (((*puzzleData) | token) != token)
                upgradeStatus = upgradeStatus == US_NO_CHANGE ? US_CLOUD_UPGRADE : US_FULL_UPGRADE;
            
            *puzzleData = merge ? (*puzzleData) | token : token;
			offset += sizeof(token);
		}
        
		++levelIndex;
	}

	CCLog("GameProgress::LevelData Done");
    
    if (upgradeStatus == US_LOCAL_UPGRADE || upgradeStatus == US_FULL_UPGRADE)
        invalidateCaches();
    return upgradeStatus;
}

const u8* GameProgress::exportData(ulong& size)
{
    int32_t dataVersion = (int32_t)kDataVersion;
    int32_t minsPlayed = (int32_t)mMinsPlayed;
    int32_t numLevels = (int32_t)getNumLevels();
    
    size_t sizeVersion = sizeof(dataVersion);
    size_t sizeMinsPlayed = sizeof(minsPlayed);
    size_t sizeHeader = sizeof(numLevels);
    size_t sizeData = numLevels * getNumPuzzlesPerLevel() * sizeof(u8);
    size_t sizeExportData = sizeVersion + sizeMinsPlayed + sizeHeader + sizeData;
    
    if (mExportData == NULL)
        mExportData = (u8*)malloc(sizeExportData);
    
    if (mExportData)
    {
        memset(mExportData, 0, sizeExportData);
        memcpy(mExportData, &dataVersion, sizeVersion);
        memcpy(mExportData + sizeVersion, &minsPlayed, sizeMinsPlayed);
        memcpy(mExportData + (sizeVersion + sizeMinsPlayed), &numLevels, sizeHeader);
        memcpy(mExportData + (sizeVersion + sizeMinsPlayed + sizeHeader), mLevelData, sizeData);
    
        size = sizeExportData;
        return mExportData;
    }
    else
    {
        size = 0;
        return NULL;
    }
}

UpgradeStatus GameProgress::upgradeToData(const u8* data, ulong size)
{
    if (data)
    {
        ulong offset = 0;
        return syncWithData(data, size, offset, true);
    }
    else
        return US_NO_CHANGE;
}

void GameProgress::load(void)
{
    FileManager::FM()->loadProgress(kFilePath, kFileExt, this, loadCallback);
}

bool GameProgress::save(void)
{
    SaveData* saveData = new SaveData(mLevelData, (int32_t)kDataVersion, (int32_t)mMinsPlayed, (int32_t)getNumLevels(), (int32_t)getNumPuzzlesPerLevel());
    if (saveData)
    {
        FileManager::FM()->saveProgress(kFilePath, kFileExt, (void*)saveData, saveCallback);
        return true;
    }
    else
        return false;
}

void loadCallback(void* arg)
{
    if (arg == NULL)
        return;
    
    FMLoadData* loadData = (FMLoadData*)arg;
    GameProgress* gp = (GameProgress*)loadData->_data;
	ulong size = loadData->_size, offset = loadData->_offset;
    CCAssert(size >= offset, "Invalid file stream pointers in GameProgress::loadCallback.");
    u8* stream = loadData->_stream;
    
    CCAssert(stream && gp, "Invalid user data in GameProgress::loadCallback.");
    if (!stream || !gp)
        return;
    
    stream += offset;
    gp->syncWithData(stream, size, offset, false);
    
    // Set loadData->_error state here if required.
    loadData->_offset = offset;
	CCLog("GameProgress: Successfully loaded game progress data. File size: %lu Stream offset: %lu", size, offset);
}

static void saveCallback(void* arg)
{
    if (arg == NULL)
        return;
    
    FMSaveData* fmSaveData = (FMSaveData*)arg;
    SaveData* saveData = (SaveData *)fmSaveData->_data;
    if (fmSaveData->_abort)
    {
        UtilsNS::CMLog("GameProgress: aborting save.");
        fmSaveData->_data = NULL;
        delete saveData, saveData = NULL;
        return;
    }
    
    FILE* file = fmSaveData->_file;
    do
    {
        fmSaveData->_error = true;
        
        if (saveData->_data == NULL)
            break;
        
        int32_t dataVersion = saveData->_dataVersion;
        if (fwrite(&dataVersion, sizeof(dataVersion), 1, file) != 1)
            break;
        
        int32_t minsPlayed = saveData->_minsPlayed;
        if (fwrite(&minsPlayed, sizeof(minsPlayed), 1, file) != 1)
        	break;

        int32_t numLevels = saveData->_numLevels;
        if (fwrite(&numLevels, sizeof(numLevels), 1, file) != 1)
            break;

        u8* puzzleData = saveData->_data;
        int numPuzzles = numLevels * saveData->_numPuzzlesPerLevel;
        if (fwrite(puzzleData, sizeof(puzzleData[0]), numPuzzles, file) != numPuzzles)
            break;
        
        fmSaveData->_error = false;
    } while (false);
    
    if (!fmSaveData->_error)
    {
        UtilsNS::CMLog("GameProgress: saved progress successfully.");
        fmSaveData->_data = NULL;
        delete saveData, saveData = NULL;
    }
    else
        UtilsNS::CMLog("GameProgress: error while saving.");
}
