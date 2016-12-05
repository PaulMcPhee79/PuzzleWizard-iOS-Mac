#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include "cocos2d.h"
#include <Utils/CMLock.h>
USING_NS_CC;

typedef void (*FM_CALLBACK)(void*);
typedef std::map<std::string, pthread_mutex_t*> MutexMap;

typedef struct FMLoadData
{
    explicit FMLoadData(const std::string& filename, void* data, u8* stream, ulong size)
    : _filename(filename), _data(data), _stream(stream), _size(size), _offset(0), _error(false)
    {

    }
    ~FMLoadData(void)
    {
        _stream = 0;
    }
    void reset(const std::string& filename, u8* stream, ulong size, ulong offset = 0)
    {
        _filename = filename;
        _stream = stream;
        _size = size;
        _offset = offset;
        _error = false;
    }
    std::string _filename;
    void* _data;
    u8* _stream;
    ulong _size;
    ulong _offset;
    bool _error;
} FMLoadData;

typedef struct FMSaveData
{
    explicit FMSaveData(const std::string& filename, FILE* file, void* data)
    : _filename(filename), _file(file), _data(data), _error(false), _abort(false)
    {
        
    }
    ~FMSaveData(void)
    {
        _data = 0;
    }
    std::string _filename;
    FILE* _file;
    void* _data;
    bool _error;
    bool _abort;
} FMSaveData;

class FileManager
{
public:
    static FileManager* FM(void);
	~FileManager(void);
    
    // Thread-safe ; asynchronous save ; save->move->backup mechanic for data integrity
    void loadProgress(std::string path, std::string pathExt, void* data, FM_CALLBACK reader);
    void saveProgress(std::string path, std::string pathExt, void* data, FM_CALLBACK writer);
    
    // Non thread-safe ; synchronous save
    void load(std::string path, void* data, FM_CALLBACK reader);
    void save(std::string path, void* data, FM_CALLBACK writer);
    void write(void* data, void* buf, size_t bufSize, FM_CALLBACK writer);
    
private:
    FileManager(void);
	FileManager(const FileManager& other);
	FileManager& operator=(const FileManager& rhs);
    pthread_mutex_t* getMutex(const char* key);
    void lockNLoad(std::string path, std::string pathExt, void* data, FM_CALLBACK reader);
    void lockNSave(std::string path, std::string pathExt, void* data, FM_CALLBACK writer);
    void loadImpl(std::string path, std::string pathExt, void* data, FM_CALLBACK reader);
    
    MutexMap mMutexes;
};

#endif // __FILE_MANAGER_H__
