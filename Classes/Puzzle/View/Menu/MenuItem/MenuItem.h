#ifndef __MENU_ITEM_H__
#define __MENU_ITEM_H__

#include "cocos2d.h"
#include <Prop/UIControls/MenuButton.h>
#include <Puzzle/View/Playfield/TileDecoration.h>
USING_NS_CC;

class MenuItem : public MenuButton
{
public:
	MenuItem(void);
	MenuItem(int fontSize);
	virtual ~MenuItem(void);

	static MenuItem* create(CCSpriteFrame* upFrame, CCSpriteFrame* downFrame = NULL, IEventListener* listener = NULL, bool autorelease = true);
	static MenuItem* create(const char* text, int fontSize = 32, IEventListener* listener = NULL, bool autorelease = true);
    virtual bool initWith(CCSpriteFrame* upFrame, CCSpriteFrame* downFrame = NULL, IEventListener* listener = NULL);
    
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
    
    virtual void setSelected(bool value);
    virtual void update(int controllerState) { }
    
    bool isIgnoreLangOffsetsEnabled(void) const { return mIgnoreLangOffsets; }
    void enableIgnoreLangOffsets(bool enable)
    {
        if (mIgnoreLangOffsets != enable)
        {
            mIgnoreLangOffsets = enable;
            repositionActiveIcon();
        }
    }
    
protected:
    virtual void updateContentBounds(void);
    virtual void repositionActiveIcon(void);
    
    bool mIgnoreLangOffsets;
    CCNode* mContents;
    TileDecoration* mSelectedIcon;
};
#endif // __MENU_ITEM_H__
