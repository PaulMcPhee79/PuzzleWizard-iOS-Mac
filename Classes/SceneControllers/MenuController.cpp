
#include "MenuController.h"
#include <SceneControllers/PlayfieldController.h>
#include <SceneViews/MenuView.h>
#include <Prop/UIControls/MenuButton.h>
#include <Puzzle/View/Menu/MenuDialog.h>
#include <Puzzle/Data/Level.h>
#include <Puzzle/View/Menu/LevelMenu.h>
#include <Puzzle/View/Menu/MenuDialogManager.h>
#include <Puzzle/Controllers/PuzzleController.h>
#include <Puzzle/View/Playfield/PuzzleBoard.h>
#include <Prop/UIControls/FXButton.h>
#include <Utils/Globals.h>
#include <Utils/Utils.h>
#include <Utils/CMTypes.h>
#include <Utils/BridgingUtility.h>
#if CM_BETA
#include <Testing/TFManager.h>
#endif
USING_NS_CC;

static const int kButtonTagNextUnsolvedPuzzle = 1001;
static const int kButtonTagMainMenu = 1002;

MenuController::MenuController(PlayfieldController* scene)
	:
	mState(MenuController::NONE),
	mView(NULL),
	mDialogManager(NULL),
	mScene(NULL)
{
	mScene = scene;
}

MenuController::~MenuController(void)
{
	CC_SAFE_RELEASE_NULL(mView);
    if (mDialogManager)
    {
        mDialogManager->removeEventListener(MenuDialogManager::EV_TYPE_DID_CLOSE_LEVEL_COMPLETED_DIALOG(), this);
        CC_SAFE_RELEASE_NULL(mDialogManager);
    }
	mScene = NULL;
}

void MenuController::setupController(void)
{
	if (mView)
		return;

	mView = new MenuView(CMGlobals::HUD, this);
	mView->init();
	mView->pushSubviewForKey("LevelMenu");
	mScene->addProp(mView);
}

void MenuController::attachEventListeners(void)
{
    
}

void MenuController::detachEventListeners(void)
{
    
}

void MenuController::applyGameSettings(void)
{
    if (mView)
    {
        LevelMenu* levelMenu = mView->getLevelMenu();
        if (levelMenu)
            levelMenu->refresh();
    }
    
    if (mDialogManager)
        mDialogManager->applyGameSettings();
}

void MenuController::enableGodMode(bool enable)
{
	if (mView && mView->getLevelMenu())
		mView->getLevelMenu()->setUnlockedAll(enable);
}

void MenuController::setState(MenuState value)
{
	if (value == mState)
        return;

    MenuState previousState = mState;

    // Clean up previous state
    switch (previousState)
    {
		case MenuController::MENU_TITLE:
			mScene->addPauseButton("menu-icon");
#ifdef CHEEKY_DESKTOP
            mScene->showEscKeyPrompt(3, 1.5f);
#endif

			if (mDialogManager == NULL)
			{
				mDialogManager = new MenuDialogManager(CMGlobals::HUD);
				mDialogManager->init();
				mDialogManager->addMenuButton("NextUnsolvedPuzzle", createNextUnsolvedButton());
				mDialogManager->addMenuButton("MainMenu", createMainMenuButton());
                mDialogManager->addEventListener(MenuDialogManager::EV_TYPE_DID_CLOSE_LEVEL_COMPLETED_DIALOG(), this);
				mScene->addProp(mDialogManager);
			}
            break;
		case MenuController::TRANSITION_IN:
            break;
		case MenuController::MENU_IN:
            mScene->popFocusState(CMInputs::FOCUS_STATE_MENU);
            break;
		case MenuController::TRANSITION_OUT:
            break;
		case MenuController::MENU_OUT:
            break;
        default:
            break;
    }

    // Apply new state
    mState = value;
    
    switch (mState)
    {
        case MenuController::MENU_TITLE:
            /*mView->populateTitleSubview();
            mView->pushSubviewForKey("Title");
            mView->fadeSubviewOverTime("Title", 1.0f, 4.0f);*/
            break;
        case MenuController::TRANSITION_IN:
            mView->setVisible(true);
            enableRateTheGameIcon(mScene->shouldPromptForRating());
            showLevelMenu();
            break;
        case MenuController::MENU_IN:
            mScene->pushFocusState(CMInputs::FOCUS_STATE_MENU);
            mScene->enableMenuMode(true);
#if CM_BETA
            if (mScene->isRestoringPreviousSession() == false)
                TFManager::TFM()->applyPendingState();
#endif
            break;
        case MenuController::TRANSITION_OUT:
            mScene->enableMenuMode(false);
            hideLevelMenu();
            break;
		case MenuController::MENU_OUT:
            mView->setVisible(false);
            break;
        default:
            break;
    }
}

FXButton* MenuController::createNextUnsolvedButton(void)
{
	FXButton* button = FXButton::create(TextUtils::FONT_FX, "Next Unsolved Puzzle", 56, this, false);
    button->setSelectedColor(0xa7ff67);
    button->setUnselectedColor(0xa7ff67);
    button->setSelectedArrowScale(1.5f);
    button->setSelected(true);
	button->setTag(kButtonTagNextUnsolvedPuzzle);
	//positionMenuButton(button);
	return button;
}

FXButton* MenuController::createMainMenuButton(void)
{
    FXButton* button = FXButton::create(TextUtils::FONT_FX, "Level Menu", 56, this, false);
    button->setSelectedColor(0xa7ff67);
    button->setUnselectedColor(0xa7ff67);
    button->setSelectedArrowScale(1.5f);
    button->setSelected(true);
	button->setTag(kButtonTagMainMenu);
	//positionMenuButton(button);
	return button;
}

void MenuController::showMenuDialog(const char* key)
{
	if (mDialogManager)
		mDialogManager->pushDialogForKey(key);
}

void MenuController::showMenuButton(const char* key)
{
	if (mDialogManager)
		mDialogManager->showMenuButton(key);
}

bool MenuController::isEscDialogShowing(void)
{
    return mDialogManager && mDialogManager->isEscDialogShowing();
}

void MenuController::showEscDialog(PlayfieldController::PfState state)
{
	if (mDialogManager)
	{
		if (state == PlayfieldController::MENU)
			mDialogManager->pushDialogForKey("MenuEsc");
		else if (state == PlayfieldController::PLAYING)
			mDialogManager->pushDialogForKey("PlayfieldEsc");
	}
}

void MenuController::hideEscDialog(PlayfieldController::PfState state)
{
	if (mDialogManager)
		mDialogManager->popDialog();
}

void MenuController::showLevelMenu(void)
{
	if (mView)
		mView->showLevelMenuOverTime(0.5f);
}

void MenuController::hideLevelMenu(void)
{
	if (mView)
		mView->hideLevelMenuOverTime(1.0f);
}

void MenuController::hideLevelMenuInstantaneously(void)
{
    if (mView)
		mView->hideLevelMenuInstantaneously();
}

void MenuController::puzzleWasSelectedAtMenu(int puzzleID)
{
	mScene->puzzleWasSelectedAtMenu(puzzleID);
}

void MenuController::populateLevelMenu(CCArray* levels)
{
	if (mView)
		mView->populateLevelMenuView(levels);
}

void MenuController::jumpLevelMenuToLevel(int levelIndex, int puzzleIndex)
{
	if (mView && mView->getLevelMenu())
		mView->getLevelMenu()->jumpToLevelIndex(levelIndex, puzzleIndex);
}

void MenuController::returnToLevelMenu(void)
{
	if (mView && mView->getLevelMenu())
		mView->getLevelMenu()->returnToLevelMenu();
}

void MenuController::refreshLevelMenu(void)
{
    if (mView && mView->getLevelMenu())
		mView->getLevelMenu()->refresh();
}

void MenuController::enableRateTheGameIcon(bool enable)
{
    if (mView)
        mView->enableRateTheGameIcon(enable);
}

void MenuController::animateRateTheGameIcon(bool enable)
{
    if (mView)
        mView->animateRateTheGameIcon(enable);
}

void MenuController::showLevelUnlockedDialog(const char* key, int levelIndex)
{
	if (mDialogManager)
	{
		mDialogManager->addLevelUnlockedDialog(key, levelIndex);
		mDialogManager->pushDialogForKey(key, true);
		mScene->playSound("level-unlocked");
	}
}

void MenuController::showLevelCompletedDialog(int levelIndex)
{
	if (mDialogManager)
	{
		mDialogManager->pushLevelCompletedDialog(levelIndex);
		mScene->playSound("level-unlocked");
	}
}

void MenuController::showPuzzleWizardDialog(void)
{
	if (mDialogManager)
	{
		mDialogManager->showPuzzleWizardDialog();
		mScene->playSound("level-unlocked");
	}
}

void MenuController::showBetaIQRatingDialog(int IQValue, std::string puzzleName)
{
    if (mDialogManager)
        mDialogManager->pushBetaIQRatingDialog(IQValue, puzzleName);
}

void MenuController::refreshColorScheme(void)
{
    if (mView && mView->getLevelMenu())
        mView->getLevelMenu()->refreshColorScheme();
    if (mDialogManager)
        mDialogManager->refreshColorBlindText();
}

void MenuController::refreshCloudText(void)
{
    if (mDialogManager)
        mDialogManager->refreshCloudText();
}

void MenuController::refreshLowPowerMode(void)
{
    if (mDialogManager)
        mDialogManager->refreshLowPowerModeText();
}

void MenuController::deviceBackClicked(void)
{
	if (mScene->getState() == PlayfieldController::MENU)
	{
		if (mDialogManager)
		{
			if (mDialogManager->isExitDialogShowing())
			{
				int kEvNo = 17;
				mDialogManager->onEvent(MenuDialog::EV_TYPE_ITEM_DID_ACTIVATE(), &kEvNo);
			}
			else if (mDialogManager->isEscDialogShowing())
				mDialogManager->popDialog();
			else
				mScene->showEscDialog();
		}
	}
	else if (mScene->getState() == PlayfieldController::PLAYING)
	{
		if (mDialogManager)
		{
			if (mDialogManager->isEscDialogShowing())
				mDialogManager->popDialog();
			else
				mScene->showEscDialog();
		}
	}
}

void MenuController::onEvent(int evType, void* evData)
{
	if (evType == LevelMenu::EV_TYPE_DID_TRANSITION_IN())
	{
		setState(MenuController::MENU_IN);
	}
	else if (evType == LevelMenu::EV_TYPE_DID_TRANSITION_OUT())
	{
		setState(MenuController::MENU_OUT);
	}
	else if (evType == LevelMenu::EV_TYPE_DEVICE_BACK_CLICKED())
	{
		deviceBackClicked();
	}
    else if (evType == LevelMenu::EV_TYPE_RATE_THE_GAME())
    {
        BridgingUtility::userRespondedToRatingPrompt(true);
        //enableRateTheGameIcon(mScene->shouldPromptForRating());
    }
	else if (evType == MenuButton::EV_TYPE_RAISED())
	{
		FXButton* button = static_cast<FXButton*>(evData);
		if (button)
		{
			switch (button->getTag())
			{
				case kButtonTagNextUnsolvedPuzzle:
					{
                        if (mDialogManager)
							mDialogManager->popAllDialogs();
						mScene->proceedToNextUnsolvedPuzzle();
					}
					break;
				case kButtonTagMainMenu:
					{
						if (mDialogManager)
							mDialogManager->popAllDialogs();
                        mScene->returnToLevelMenu();
					}
					break;
			}
		}
	}
    else if (evType == MenuDialogManager::EV_TYPE_DID_CLOSE_LEVEL_COMPLETED_DIALOG())
    {
        dispatchEvent(evType);
    }
    else if (evType == MenuView::EV_TYPE_SPLASH_DID_HIDE())
    {
        mScene->splashViewDidHide();
    }
}

void MenuController::advanceTime(float dt)
{
	if (mView)
		mView->advanceTime(dt);
	if (mDialogManager)
		mDialogManager->advanceTime(dt);
}
