
#include "Utils.h"
#include "ccMacros.h"
#include <cmath>
#include <functional>
#include <cctype>
#include <locale>
USING_NS_CC;

namespace CMUtils
{
    static bool s_randSeeded = false;
    int nextRandom(int min, int max)
    {
        if (!s_randSeeded)
        {
            srand((unsigned int)time(0));
            s_randSeeded = true;
            rand(); // First rand is always the same, so throw it away.
        }
        
        if (min > max)
            return 0;
        else
            return rand() % ((max - min) + 1) + min;
    }
    
	void setOpacity(CCObject* object, GLubyte opacity)
	{
		CCArray* children = NULL;
		CCObject* child = NULL;
		CCNode* nodeObject = static_cast<CCNode*>(object);
		if (nodeObject)
			children = nodeObject->getChildren();
		if (children)
		{
			CCARRAY_FOREACH(children, child)
			{
				CCRGBAProtocol *pRGBAProtocol = dynamic_cast<CCRGBAProtocol*>(child);
				if (pRGBAProtocol)
					pRGBAProtocol->setOpacity(opacity);

				CCNode* singleObject = static_cast<CCNode*>(child);
				if (singleObject && singleObject->getChildrenCount() > 0)
					setOpacity(singleObject, opacity);
			}
		}
	}    

	CCSprite* createColoredQuad(const CCSize& size)
	{
		CCSprite* quad = new CCSprite();
		quad->init();
		quad->setTextureRect(CCRectMake(0, 0, size.width, size.height));
		quad->setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey("ColoredQuad"));
		quad->autorelease();
		return quad;
	}

	CCRect boundsInSpace(CCNode* space, CCNode* target)
	{
		if (space == NULL || target == NULL || target->getParent() == NULL)
			return CCRectZero;

		CCRect targetBounds = target->boundingBox();
		CCPoint pointInSpace = space->convertToNodeSpace(target->getParent()->convertToWorldSpace(target->getPosition()));
		CCRect bounds = CCRectMake(
			pointInSpace.x - targetBounds.size.width / 2,
			pointInSpace.y - targetBounds.size.height / 2,
			targetBounds.size.width,
			targetBounds.size.height);
		return bounds;
	}

	CCRect intersectionRect(const CCRect& rectA, const CCRect& rectB)
	{
		float left = MAX(rectA.getMinX(), rectB.getMinX());
		float right = MIN(rectA.getMinX() + rectA.size.width, rectB.getMinX() + rectB.size.width);
		float top = MAX(rectA.getMinY(), rectB.getMinY());
		float bottom = MIN(rectA.getMinY() + rectA.size.height, rectB.getMinY() + rectB.size.height);

		if (left > right || top > bottom)
			return CCRectMake(0, 0, 0, 0);
		else
			return CCRectMake(left, top, right - left, bottom - top);
	}

	CCRect unionRect(const CCRect& rectA, const CCRect& rectB)
	{
		float left = MIN(rectA.getMinX(), rectB.getMinX());
		float right = MAX(rectA.getMinX() + rectA.size.width, rectB.getMinX() + rectB.size.width);
		float top = MIN(rectA.getMinY(), rectB.getMinY());
		float bottom = MAX(rectA.getMinY() + rectA.size.height, rectB.getMinY() + rectB.size.height);
		return CCRectMake(left, top, right - left, bottom - top);
	}

	CCPoint moveInCircle(float gameTime, float speed)
	{
		float time = gameTime * speed;
		return CCPointMake(cos(time), sin(time));
	}

	ccColor3B uint2color3B(uint color)
	{
		return ccc3((GLubyte)((color >> 16) & 0xff), (GLubyte)((color >> 8) & 0xff), (GLubyte)(color & 0xff));
	}

	uint color3B2uint(const ccColor3B& color)
	{
		return ((uint)color.r << 16) + ((uint)color.g << 8) + (uint)color.b;
	}
    
//    std::string ltrim(const std::string &s)
//    {
//        std::string trimmed(s);
//        trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(), std::not1(std::ptr_fun<int, int>(::isspace))));
//        return trimmed;
//    }
//
//    std::string rtrim(const std::string &s)
//    {
//        std::string trimmed(s);
//        trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(), std::not1(std::ptr_fun<int, int>(::isspace))).base(), trimmed.end());
//        return trimmed;
//    }
//
//    std::string trim(const std::string &s)
//    {
//        return ltrim(rtrim(s));
//    }

    std::string trim(const std::string &s)
    {
    	std::string trimmed(s);
    	trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    	trimmed.erase(0, trimmed.find_last_not_of(" \t")+1);
        return trimmed;
    }

    std::string toLowerEN(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }
}

