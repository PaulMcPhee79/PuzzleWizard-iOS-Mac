#ifndef __MENU_DIALOG_MANAGER_H__
#define __MENU_DIALOG_MANAGER_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IInteractable.h>
#include <Interfaces/IResDependent.h>
#include <Interfaces/ILocalizable.h>
#include <Puzzle/View/Menu/MenuDialog.h>
#include <Utils/BridgingUtility.h>
class MenuButton;
class FXButton;
class ByteTweener;
USING_NS_CC;

class MenuDialogManager : public Prop, public EventDispatcher, public IEventListener, public IInteractable, public IResDependent, public ILocalizable
{
public:
    static int EV_TYPE_DID_CLOSE_LEVEL_COMPLETED_DIALOG() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
	MenuDialogManager(int category);
	virtual ~MenuDialogManager(void);
	virtual bool init(void);

    void applyGameSettings(void);
    virtual void resolutionDidChange(void);
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
    bool isEscDialogShowing(void);
    bool isExitDialogShowing(void);
    int getStackHeight(void) const;
	void refreshMusicText(void);
	void refreshSfxText(void);
    void refreshMusicGauge(void);
    void refreshSfxGauge(void);
    void refreshDisplayModeHighlight(void);
    void refreshCloudText(void);
    void refreshColorBlindText(void);
    void refreshLanguageText(void);
    void refreshLowPowerModeText(void);
	void addLevelUnlockedDialog(const char* key, int levelIndex);
    void pushCloudQueryDialog(void);
	void pushLevelCompletedDialog(int levelIndex, bool animate = true);
    void pushBetaIQRatingDialog(int IQValue, std::string puzzleName);
	void showPuzzleWizardDialog(void);
    void addRateTheGameDialog(const char* key);
	void addDialog(const char* key, MenuDialog* dialog);
	void removeDialog(const char* key);
	void pushDialogForKey(const char* key, bool animate = false);
	void popDialog(void);
	void popAllDialogs(void);

	void addMenuButton(const char* key, FXButton* button);
	void removeMenuButton(const char* key);
	void showMenuButton(const char* key);
	void hideMenuButton(const char* key);

	virtual void onEvent(int evType, void* evData);
    virtual unsigned int getInputFocus(void) const { return CMInputs::HAS_FOCUS_MENU_DIALOG; }
    virtual void willLoseFocus(void);
    virtual void update(int controllerState);
	virtual void advanceTime(float dt);

private:
    Localizer::LocaleType evLang2Locale(int evLang);
    int locale2EvLang(Localizer::LocaleType locale);
	MenuDialog* getCurrentDialog(void) const;
	FXButton* getMenuButton(void) const { return mMenuButton; }
	void setMenuButton(FXButton* value);
	void updateMenuButton(void);
    void positionMenuButton(FXButton* button);
	bool isTouchEnabled(void) const;
    bool isDialogOnStack(MenuDialog* dialog);
    void pushDialogForKey(const char* key, MenuDialog::AnimState anim);

    std::string getKeyForDialog(MenuDialog* dialog);
	MenuDialog* getDialogForKey(const char* key);
    MenuDialog* createCloudQueryDialog(void);
	MenuDialog* createLevelCompletedDialog(int levelIndex);

    int mBetaIQRating;
    std::string mBetaPuzzleName;
    
    int mQueryKey;
	std::map<std::string, MenuDialog*> mMenuDialogs;
	std::vector<MenuDialog*> mMenuDialogStack;

	FXButton* mMenuButton;
	std::map<std::string, FXButton*> mMenuButtons;

	CCSprite* mBackDrop;
	ByteTweener* mBackDropTweener;
};
#endif // __MENU_DIALOG_MANAGER_H__
