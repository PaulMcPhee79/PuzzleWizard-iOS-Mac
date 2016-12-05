#ifndef __CMLABEL_BMFONT_H__
#define __CMLABEL_BMFONT_H__

#include "cocos2d.h"
USING_NS_CC;

class CMLabelBMFont : public CCLabelBMFont
{
public:
	CMLabelBMFont();
	virtual ~CMLabelBMFont(void);

    static CMLabelBMFont* create(const char *str, const char *fntFile, float width, CCTextAlignment alignment, CCPoint imageOffset);
	static CMLabelBMFont* create(const char *str, const char *fntFile, float width, CCTextAlignment alignment);
	static CMLabelBMFont* create(const char *str, const char *fntFile, float width);
	static CMLabelBMFont* create(const char *str, const char *fntFile);
    static CMLabelBMFont* create();
    
    virtual void setString(unsigned short *newString, bool needUpdateLabel);
};
#endif // __CMLABEL_BMFONT_H__
