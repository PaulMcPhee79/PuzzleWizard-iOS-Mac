#ifndef __UTILS_H__
#define __UTILS_H__

#include "cocos2d.h"
#include <algorithm>
USING_NS_CC;

namespace CMUtils
{
	template<typename T, size_t N>
	std::vector<T> makeVector(const T (&data)[N])
	{
		return std::vector<T>(data, data+N);
	}
    
    template<size_t N>
	std::vector<std::string> makeStrVector(const char* (&data)[N])
	{
		return std::vector<std::string>(data, data+N);
	}
    
    template<typename T, size_t N>
    std::map<T, T> makeMap(const T (&data)[N])
    {
        CCAssert((N & 1) == 0, "CMUtils::makeMap - requires even number of parameters.");
        std::map<T, T> m;
        for (int i = 0; i < N-1; i+=2)
            m[data[i]] = data[i+1];
        return m;
    }
    
    template<size_t N>
    std::map<std::string, std::string> makeStrMap(const char* (&data)[N])
    {
        CCAssert((N & 1) == 0, "CMUtils::makeStrMap - requires even number of parameters.");
        std::map<std::string, std::string> m;
        for (int i = 0; i < N-1; i+=2)
            m[data[i]] = data[i+1];
        return m;
    }
    
	void setOpacity(CCObject* object, GLubyte opacity);
	CCSprite* createColoredQuad(const CCSize& size);
	CCRect boundsInSpace(CCNode* space, CCNode* target);
	CCRect intersectionRect(const CCRect& rectA, const CCRect& rectB);
	CCRect unionRect(const CCRect& rectA, const CCRect& rectB);
	CCPoint moveInCircle(float gameTime, float speed);
    ccColor3B uint2color3B(uint color);
	uint color3B2uint(const ccColor3B& color);
    int nextRandom(int min, int max); // Inclusive
    
//    std::string ltrim(const std::string &s);
//    std::string rtrim(const std::string &s);
    std::string trim(const std::string &s);
    std::string toLowerEN(std::string s);
    
    // Templates
    template<typename T>
	std::string strConcatVal(const char* str, T val)
    {
        std::ostringstream oss;
		oss << str << val;
		return oss.str();
    }
    
	template<typename Container>
	void delete_them(Container& c) { while(!c.empty()) delete c.back(), c.pop_back(); }
    
    template<typename Container>
    void release_them(Container& c) { while(!c.empty()) { CC_SAFE_RELEASE(c.back()); c.pop_back(); } }
    
    template<typename T>
    bool contains(std::vector<T*>& vec, T* t)
    {
        typename std::vector<T*>::iterator findIt = std::find(vec.begin(), vec.end(), t);
        return findIt != vec.end();
    }
    
    template<typename T>
    int indexOf(std::vector<T*>& vec, T* t)
    {
        for (int i = 0; i < vec.size(); ++i)
        {
            if (vec[i] == t)
                return i;
        }
        
        return -1;
    }
    
    template<typename T>
    void erase(std::vector<T*>& vec, T* t)
    {
        if (CMUtils::contains(vec, t))
            vec.erase(std::remove(vec.begin(), vec.end(), t));
    }
}

#endif // __UTILS_H__
