#ifndef __MENU_CONTROLLER_H__
#define __MENU_CONTROLLER_H__

#include "cocos2d.h"
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <SceneControllers/PlayfieldController.h>
class MenuView;
class MenuButton;
class Level;
class MenuDialogManager;
class FXButton;
USING_NS_CC;

class MenuController : public CCObject, public EventDispatcher, public IEventListener
{
public:
	enum MenuState { NONE = 0, MENU_TITLE, TRANSITION_IN, MENU_IN, TRANSITION_OUT, MENU_OUT };

	MenuController(PlayfieldController* scene);
	virtual ~MenuController(void);

	void setupController(void);
	void attachEventListeners(void);
	void detachEventListeners(void);
	void applyGameSettings(void);

	void enableGodMode(bool enable);
	void showMenuDialog(const char* key);
	void showMenuButton(const char* key);
    bool isEscDialogShowing(void);
	void showEscDialog(PlayfieldController::PfState state);
    void hideEscDialog(PlayfieldController::PfState state);

	void showLevelUnlockedDialog(const char* key, int levelIndex);
	void showLevelCompletedDialog(int levelIndex);
	void showPuzzleWizardDialog(void);
    void showBetaIQRatingDialog(int IQValue, std::string puzzleName);
    void refreshColorScheme(void);
    void refreshCloudText(void);
    void refreshLowPowerMode(void);
	void puzzleWasSelectedAtMenu(int puzzleID);
	void populateLevelMenu(CCArray* levels);
	void jumpLevelMenuToLevel(int levelIndex, int puzzleIndex);
	void returnToLevelMenu(void);
    void refreshLevelMenu(void);
    void enableRateTheGameIcon(bool enable);
    void animateRateTheGameIcon(bool enable);
    void hideLevelMenuInstantaneously(void);

	virtual void onEvent(int evType, void* evData);
	void advanceTime(float dt);

	MenuState getState(void) const { return mState; }
	void setState(MenuState value);

private:
	FXButton* createNextUnsolvedButton(void);
	FXButton* createMainMenuButton(void);
	void positionMenuButton(FXButton* button);
	void showLevelMenu(void);
	void hideLevelMenu(void);
	void deviceBackClicked(void);

	MenuState mState;
	MenuView* mView;
	MenuDialogManager* mDialogManager;
	PlayfieldController* mScene;
};
#endif // __MENU_CONTROLLER_H__
