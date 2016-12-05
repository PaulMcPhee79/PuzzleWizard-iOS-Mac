#ifndef __IPLAYER_VIEW_H__
#define __IPLAYER_VIEW_H__

class IPlayerView
{
public:
	virtual ~IPlayerView(void) { }
	virtual void playerValueDidChange(uint code, int value) = 0;
	virtual void willBeginMoving(void) { };
    virtual void didFinishMoving(void) { };
    virtual void didIdle(void) { }
    virtual void didTreadmill(void) { }
};
#endif // __IPLAYER_VIEW_H__
