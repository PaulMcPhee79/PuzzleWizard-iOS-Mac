#ifndef __ITILE_VIEW_H__
#define __ITILE_VIEW_H__

class ITileView
{
public:
	virtual ~ITileView(void) { }
	virtual void tilePropertiesDidChange(uint code) = 0;
	virtual void tileModifiersDidChange(void) = 0;
};
#endif // __ITILE_VIEW_H__
