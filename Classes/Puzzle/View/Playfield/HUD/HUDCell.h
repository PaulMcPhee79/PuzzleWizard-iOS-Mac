#ifndef __HUD_CELL_H__
#define __HUD_CELL_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Interfaces/ILocalizable.h>
USING_NS_CC;

class HUDCell : public Prop, public ILocalizable
{
public:
	HUDCell(void);
	virtual ~HUDCell(void);

	static HUDCell* create(bool autorelease = true);
	virtual bool init(void);
    
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);

	const char* getString(void) { return mLabel ? mLabel->getString() : NULL; }
	void setString(const char* value);
	uint getColor(void) const;
	void setColor(uint value);
	void setIcon(uint evCode);
	
private:
	int mEvCode;
	CCSprite* mIcon;
	CCLabelBMFont* mLabel;
};
#endif // __HUD_CELL_H__
