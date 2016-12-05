#ifndef __ICON_BUTTON_H__
#define __ICON_BUTTON_H__

#include "cocos2d.h"
#include "FXButton.h"
USING_NS_CC;

class IconButton : public FXButton
{
public:
	explicit IconButton(TextUtils::CMFontType fontType);
	virtual ~IconButton(void);
    
	static IconButton* create(TextUtils::CMFontType fontType, const char* text, int fontSize, CCSpriteFrame* iconFrame, IEventListener* listener = NULL, bool autorelease = true);
	virtual bool initWith(const char* text, int fontSize, CCSpriteFrame* iconFrame, IEventListener* listener = NULL);
    
    CCSize getIconSize(void) const;
    
    void setSelectedColors(uint iconColor, uint fontColor);
	void setUnselectedColors(uint iconColor, uint fontColor);
    
    void setIconOffset(const CCPoint& offset);
    
protected:
    virtual void repositionSelectedArrow(void);
    virtual void layoutContents(void);
    virtual void updateContentColor(void);
    virtual void updateContentBounds(void);

private:
    uint mSelectedIconColor;
    uint mUnselectedIconColor;
    CCPoint mIconOffset;
    CCSprite* mIcon;
};
#endif // __ICON_BUTTON_H__
