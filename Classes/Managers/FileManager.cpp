
#include "FileManager.h"
#include <Utils/UtilsNS.h>
#include <Utils/fmemopen.h>
#include <errno.h>

typedef struct FMAsyncData
{
    explicit FMAsyncData(pthread_mutex_t* mutex, void* data)
    : _mutex(mutex), _data(data)
    {
        
    }
    
    pthread_mutex_t* _mutex;
    void* _data;
} FMAsyncData;

typedef struct FMClientData
{
    explicit FMClientData(std::string basePath, std::string path, std::string pathExt, void* data, FM_CALLBACK cb)
    : _basePath(basePath), _path(path), _pathExt(pathExt), _data(data), _cb(cb)
    {
        
    }
    
    std::string _basePath;
    std::string _path;
    std::string _pathExt;
    void* _data;
    FM_CALLBACK _cb;
} FMClientData;

static const char* kPathModifierOld = "_backup";
static const char* kPathModifierNew = "_new";

static void saveImpl(void* data);
static void* saveAsync(void* data);

static FileManager *g_sharedFileManager = NULL;

FileManager* FileManager::FM(void)
{
    if (!g_sharedFileManager)
	{
		g_sharedFileManager = new FileManager();
	}
	return g_sharedFileManager;
}

FileManager::FileManager(void)
{
    CCAssert(g_sharedFileManager == NULL, "Attempted to allocate a second instance of a singleton.");
}

FileManager::~FileManager(void)
{
    for (MutexMap::iterator it = mMutexes.begin(); it != mMutexes.end(); ++it)
    {
        { CMLock lock(it->second, true); }
        pthread_mutex_destroy(it->second);
        free(it->second);
    }
    
    mMutexes.clear();
}

pthread_mutex_t* FileManager::getMutex(const char* key)
{
    MutexMap::iterator it = mMutexes.find(key);
    if (it != mMutexes.end())
        return it->second;
    else
    {
        size_t mutexSize = sizeof(pthread_mutex_t);
        pthread_mutex_t* mutex = (pthread_mutex_t*)malloc(mutexSize);
        if (mutex)
        {
            memset(mutex, 0, mutexSize);
            if (pthread_mutex_init(mutex, NULL) == 0)
                mMutexes[key] = mutex;
            else
            {
                free(mutex), mutex = 0;
            }
        }
        
        return mutex;
    }
}

void FileManager::loadImpl(std::string path, std::string pathExt, void* data, FM_CALLBACK reader)
{
    ulong size = 0;
    u8* stream = 0;
    std::string basePath = CCFileUtils::sharedFileUtils()->getWritablePath();
    std::string defaultPath = std::string(basePath).append(path).append(pathExt);
    FMLoadData loadData("", data, stream, size);
    
    const int kNumAltPaths = 3;
    std::string altPaths[kNumAltPaths] =
    {
        std::string(basePath).append(path).append(kPathModifierNew).append(pathExt),
        defaultPath,
        std::string(basePath).append(path).append(kPathModifierOld).append(pathExt),
    };
    
    for (int i = 0; i < kNumAltPaths; ++i)
    {
        if (stream && !loadData._error)
            break;
        
        delete[] stream;
        stream = CCFileUtils::sharedFileUtils()->getFileData(altPaths[i].c_str(), "rb", &size);
        if (stream)
        {
            loadData.reset(altPaths[i], stream, size);
            reader((void*)&loadData);
            
            if (!loadData._error && altPaths[i].compare(defaultPath) != 0)
            {
                if (rename(altPaths[i].c_str(), defaultPath.c_str()))
                {
                    std::string errStr = std::string("FileManager load error: ").append(strerror(errno)).append(". Failed to rename ").append(altPaths[i]).c_str();
                    CCLog("%s", errStr.c_str());
                }
            }
        }
    }
    
    if (!stream || loadData._error)
        CCLog("GameProgress: Could not open game progress data from path %s\n.", defaultPath.c_str());
    
    delete[] stream;
}

void FileManager::lockNLoad(std::string path, std::string pathExt, void* data, FM_CALLBACK reader)
{
	CCFileUtils::sharedFileUtils()->setPopupNotify(false);
	std::string strPath = CCFileUtils::sharedFileUtils()->getWritablePath().append(path).append(pathExt);
    
    pthread_mutex_t* mutex = getMutex(strPath.c_str());
    if (mutex)
    {
        CMLock lock(mutex, true);
        if (lock.isLocked())
        {
            loadImpl(path, pathExt, data, reader);
        }
        else
        {
            CCLog("GameProgress: Could not attain mutex lock. Load aborted.");
            return;
        }
    }
    else
    {
        // If mutexes cannot be initialized for some reason, then we're very likely also saving synchronously, so just load without locking.
        loadImpl(path, pathExt, data, reader);
    }
}

static void saveImpl(void* data)
{
    FMClientData* clientData = (FMClientData*)data;
    std::string basePath = clientData->_basePath;
	std::string newPath = std::string(basePath).append(clientData->_path).append(kPathModifierNew).append(clientData->_pathExt);
    
    FILE* file = fopen(newPath.c_str(), "wb");
    if (file)
    {
        FMSaveData fmSaveData(newPath, file, clientData->_data);
        
        // 1. Save game to path + "new".
        clientData->_cb((void*)&fmSaveData);
        
        if (fclose(file) == 0)
        {
            if (!fmSaveData._error)
            {
                // 2. Delete path + "old".
                std::string oldPath = std::string(basePath).append(clientData->_path).append(kPathModifierOld).append(clientData->_pathExt);
                if (remove(oldPath.c_str()))
                    UtilsNS::CMLog(std::string("2. FileManager save error: ").append(strerror(errno)).append(". Failed to remove ").append(oldPath).c_str());
                
                // 3. Rename path to path + "old".
                std::string origPath = std::string(basePath).append(clientData->_path).append(clientData->_pathExt);
                if (rename(origPath.c_str(), oldPath.c_str()))
                    UtilsNS::CMLog(std::string("3. FileManager save error: ").append(strerror(errno)).append(". Failed to rename ").append(origPath).c_str());
                
                // 4. Rename path + "new" to path.
                if (rename(newPath.c_str(), origPath.c_str()))
                    UtilsNS::CMLog(std::string("4. FileManager save error: ").append(strerror(errno)).append(". Failed to rename ").append(newPath).c_str());
                
                return;
            }
        }
    }
    else
    {
        // Notify client of failure
        FMSaveData fmSaveData("", NULL, clientData->_data);
        fmSaveData._abort = true;
        clientData->_cb((void*)&fmSaveData);
    }

    UtilsNS::CMLog(std::string("FileManager: Invalid save destination: ").append(newPath).c_str());
}

static void* saveAsync(void* data)
{
    if (data)
    {
        FMAsyncData* asyncData = (FMAsyncData*)data;
        FMClientData* clientData = (FMClientData*)asyncData->_data;
        
        CMLock lock(asyncData->_mutex, true);
        if (lock.isLocked())
            saveImpl((void*)clientData);
        else
        {
            // Notify client of failure
            FMSaveData fmSaveData("", NULL, clientData->_data);
            fmSaveData._abort = true;
            clientData->_cb((void*)&fmSaveData);
        }
        
        delete clientData, clientData = NULL;
        delete asyncData, asyncData = NULL;
    }
    
    return NULL;
}

void FileManager::lockNSave(std::string path, std::string pathExt, void* data, FM_CALLBACK writer)
{
    CCFileUtils::sharedFileUtils()->setPopupNotify(false);
    std::string basePath = CCFileUtils::sharedFileUtils()->getWritablePath();
	std::string strPath = std::string(basePath).append(path).append(pathExt);
    
    pthread_mutex_t* mutex = getMutex(strPath.c_str());
    
    if (mutex)
    {
        do {
            FMClientData* clientData = new FMClientData(basePath, path, pathExt, data, writer);
            FMAsyncData* asyncData = new FMAsyncData(mutex, (void *)clientData);
            
            if (!clientData || !asyncData)
                break;
            
            pthread_t threadId;
            pthread_attr_t attr;
            
            int err = pthread_attr_init(&attr);
            if (err != 0) break;
            
            err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            if (err != 0) break;
            
            err = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
            if (err != 0) break;
            
            err = pthread_create(&threadId, &attr, saveAsync, (void*)asyncData);
            if (err != 0)
            {
                // FIXME: Are we sure the thread doesn't run when err != 0? If it can and does, then one thread will be deleting a stray pointer...
                delete asyncData, asyncData = NULL;
                delete clientData, clientData = NULL;
                break;
            }
            else
            {
                CCLog("FileManager: Async save launched successfully.");
                return;
            }
        } while (false);
    }
    
    // If mutexes cannot be initialized for some reason, then prefer to save synchronously than not at all.
    FMClientData clientData(basePath, path, pathExt, data, writer);
    saveImpl((void*)&clientData);
}

void FileManager::loadProgress(std::string path, std::string pathExt, void* data, FM_CALLBACK reader)
{
    CCAssert(reader, "FileManager::loadProgress called with invalid arguments.");
    if (reader)
        lockNLoad(path, pathExt, data, reader);
}

void FileManager::saveProgress(std::string path, std::string pathExt, void* data, FM_CALLBACK writer)
{
    CCAssert(writer, "FileManager::saveProgress called with invalid arguments.");
    if (writer)
        lockNSave(path, pathExt, data, writer);
}

void FileManager::load(std::string path, void* data, FM_CALLBACK reader)
{
    CCAssert(reader, "FileManager::load called with invalid arguments.");
    if (reader)
    {
        CCFileUtils::sharedFileUtils()->setPopupNotify(false);
        std::string strPath = CCFileUtils::sharedFileUtils()->getWritablePath().append(path);
        
        ulong size = 0;
        u8* stream = CCFileUtils::sharedFileUtils()->getFileData(strPath.c_str(), "rb", &size);
        if (stream)
        {
            FMLoadData loadData(strPath, data, stream, size);
            reader((void*)&loadData);
            delete[] stream, stream = 0;
        }
    }
}

void FileManager::save(std::string path, void* data, FM_CALLBACK writer)
{
    CCAssert(writer, "FileManager::save called with invalid arguments.");
    if (writer)
    {
        CCFileUtils::sharedFileUtils()->setPopupNotify(false);
        std::string strPath = CCFileUtils::sharedFileUtils()->getWritablePath().append(path);
        FILE* file = fopen(strPath.c_str(), "wb");
        if (file)
        {
            FMSaveData fmSaveData(strPath, file, data);
            writer((void*)&fmSaveData);
            fclose(file);
        }
    }
}

void FileManager::write(void* data, void* buf, size_t bufSize, FM_CALLBACK writer)
{
    CCAssert(data && buf && bufSize > 0 && writer, "FileManager::write called with invalid arguments.");
    if (data && buf && bufSize > 0 && writer)
    {
        FILE* stream = fmemopen(buf, bufSize, "w");
        if (stream)
        {
            FMSaveData fmSaveData("", stream, data);
            writer((void*)&fmSaveData);
            fclose(stream);
        }
    }
}
