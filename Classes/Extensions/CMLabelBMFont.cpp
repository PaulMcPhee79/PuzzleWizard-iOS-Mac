
#include "CMLabelBMFont.h"
USING_NS_CC;

static const char kDefaultBMChar = '?';

CMLabelBMFont::CMLabelBMFont()
{

}

CMLabelBMFont::~CMLabelBMFont(void)
{

}

CMLabelBMFont* CMLabelBMFont::create()
{
    CMLabelBMFont * pRet = new CMLabelBMFont();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

CMLabelBMFont* CMLabelBMFont::create(const char *str, const char *fntFile)
{
    return CMLabelBMFont::create(str, fntFile);
}

CMLabelBMFont* CMLabelBMFont::create(const char *str, const char *fntFile, float width)
{
    return CMLabelBMFont::create(str, fntFile, width);
}

CMLabelBMFont* CMLabelBMFont::create(const char *str, const char *fntFile, float width, CCTextAlignment alignment)
{
    return CMLabelBMFont::create(str, fntFile, width, alignment);
}

CMLabelBMFont* CMLabelBMFont::create(const char *str, const char *fntFile, float width, CCTextAlignment alignment, CCPoint imageOffset)
{
    CMLabelBMFont *pRet = new CMLabelBMFont();
    if(pRet && pRet->initWithString(str, fntFile, width, alignment, imageOffset))
    {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

void CMLabelBMFont::setString(unsigned short *newString, bool needUpdateLabel)
{
    // Replace chars not found in charset with default char
    if (newString)
    {
        CCBMFontConfiguration* fontConfig = getConfiguration();
        if (fontConfig)
        {
            std::set<unsigned int> *charSet = m_pConfiguration->getCharacterSet();
            if (charSet)
            {
                unsigned short *it = newString;
                while (*it)
                {
                    if (*it != '\n')
                    {
                        std::set<unsigned int>::iterator findIt = charSet->find(*it);
                        if (findIt == charSet->end())
                            *it = kDefaultBMChar;
                    }
                    ++it;
                }
            }
        }
    }
    
    CCLabelBMFont::setString(newString, needUpdateLabel);
}
