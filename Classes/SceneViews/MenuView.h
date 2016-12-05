#ifndef __MENU_VIEW_H__
#define __MENU_VIEW_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/IInteractable.h>
#include <Interfaces/IResDependent.h>
class MenuController;
class MenuSubview;
class LevelMenu;
USING_NS_CC;

class MenuView : public Prop, public IEventListener, public IInteractable, public IResDependent
{
public:
    static int EV_TYPE_SPLASH_DID_HIDE() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
	MenuView(int category, MenuController* controller);
	virtual ~MenuView(void);

	virtual bool init(void);
	void attachEventListeners(void);
	void detachEventListeners(void);
    
    virtual void resolutionDidChange(void);

	LevelMenu* getLevelMenu(void) const { return mLevelMenu; }
	MenuSubview* getCurrentSubview(void) const;
	MenuSubview* subviewForKey(const char* key);
	MenuSubview* subviewForTag(int tag);
	void pushSubviewForKey(const char* key);
	void popSubview(void);
	void popAllSubviews(void);
	void destroySubviewForKey(const char* key);
	void populateTitleSubview(void);
	void fadeSubviewOverTime(const char* key, float duration, float delay = 0.0f, bool destroy = true);
	void showLevelMenuOverTime(float duration);
	void hideLevelMenuOverTime(float duration);
    void hideLevelMenuInstantaneously(void);
    void enableRateTheGameIcon(bool enable);
    void animateRateTheGameIcon(bool enable);
	void populateLevelMenuView(CCArray* levels);
	void unpopulateLevelMenuView(void);
	virtual void onEvent(int evType, void* evData);
    virtual unsigned int getInputFocus(void) const { return CMInputs::HAS_FOCUS_MENU_ALL; }
    virtual void update(int controllerState);
	virtual void advanceTime(float dt);

private:
	void pushSubview(MenuSubview* subview);
	void destroySubview(MenuSubview* subview);
	std::string keyForTag(int tag);

    float mSplashTimer;
    CCSprite* mBgQuad;
    CCNode* mSplashNode;
    Prop* mSplashProp;
	LevelMenu* mLevelMenu;
	MenuController* mController;
    
    std::map<std::string, MenuSubview*> mSubviews;
	std::vector<MenuSubview*> mSubviewStack;
};
#endif // __MENU_VIEW_H__
