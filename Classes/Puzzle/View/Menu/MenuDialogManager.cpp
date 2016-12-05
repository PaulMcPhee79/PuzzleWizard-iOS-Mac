
#include "MenuDialogManager.h"
#include <Puzzle/View/Menu/MenuBuilder.h>
#include <Prop/UIControls/MenuButton.h>
#include <Prop/UIControls/FXButton.h>
#include <Puzzle/View/Menu/LevelMenu.h>
#include <Puzzle/View/Playfield/PuzzleBoard.h>
#include <Puzzle/View/Menu/MenuItem/GaugeMenuItem.h>
#include <SceneControllers/PlayfieldController.h>
#include <Puzzle/Controllers/PuzzleController.h>
#include <Managers/ControlsManager.h>
#include <Puzzle/Inputs/TouchPad.h>
#include <Utils/ByteTweener.h>
#include <Utils/BridgingUtility.h>
#if CM_BETA
#include <Testing/TFManager.h>
#endif
#include <Prop/UIControls/IQSlider.h>
#include <Utils/Utils.h>
#include <algorithm>
#include <string>
USING_NS_CC;

typedef std::map<std::string, MenuDialog*> DlgMap;
typedef std::vector<MenuDialog*> DlgStack;
typedef std::map<std::string, FXButton*> BtnMap;

static const float kTouchPadding = 50.0f;

static const float kBackdropTweenerDuration = 0.5f;
static const GLubyte kBackdropMaxOpacity = 190; //200;

static const int kEvNull = 0;
static const int kEvResume = 1;
static const int kEvOptions = 2;
static const int kEvResetPuzzle = 3;
static const int kEvMenu = 4;
static const int kEvMusic = 5;
static const int kEvSfx = 6;
static const int kEvDisplay = 7;
static const int kEvColorBlind = 8;
static const int kEvWindowed = 9;
static const int kEvFullscreen = 10;
static const int kEvCredits = 11;
static const int kEvBack = 12;
static const int kEvLevelUnlocked = 13;
static const int kEvLevelCompleted = 14;
static const int kEvPuzzleWizard = 15;
static const int kEvYes = 16;
static const int kEvNo = 17; // HACK: Duplicate change in MenuController.cpp (search for kEvNo)
static const int kEvRateYes = 18;
static const int kEvRateNo = 19;
static const int kEvGodMode = 20;
static const int kEvCloud = 21;
static const int kEvExit = 22;
static const int kEvBetaReportBug = 23;
static const int kEvBetaIQRatingSubmitted = 24;
static const int kEvPrivacy = 25;
static const int kEvLowPowerMode = 26;

static const int kEvLang = 120;
static const int kEvLangEn = kEvLang + 1;
static const int kEvLangCn = kEvLang + 2;
static const int kEvLangDe = kEvLang + 3;
static const int kEvLangEs = kEvLang + 4;
static const int kEvLangFr = kEvLang + 5;
static const int kEvLangIt = kEvLang + 6;
static const int kEvLangJp = kEvLang + 7;
static const int kEvLangKr = kEvLang + 8;

#ifdef CHEEKY_DESKTOP
static const int kNumEvLangs = 8;
static const int kEvLangs[kNumEvLangs] = { kEvLangEn, kEvLangCn, kEvLangDe, kEvLangEs, kEvLangFr, kEvLangIt, kEvLangJp, kEvLangKr };
#endif

Localizer::LocaleType MenuDialogManager::evLang2Locale(int evLang)
{
    Localizer::LocaleType locale = Localizer::EN;
    
    switch (evLang)
    {
        case kEvLangCn: locale = Localizer::CN; break;
        case kEvLangDe: locale = Localizer::DE; break;
        case kEvLangEs: locale = Localizer::ES; break;
        case kEvLangFr: locale = Localizer::FR; break;
        case kEvLangIt: locale = Localizer::IT; break;
        case kEvLangJp: locale = Localizer::JP; break;
        case kEvLangKr: locale = Localizer::KR; break;
        case kEvLangEn:
        default: locale = Localizer::EN; break;
    }
    
    return locale;
}

int MenuDialogManager::locale2EvLang(Localizer::LocaleType locale)
{
    int evLang = kEvLangEn;
    
    switch (locale)
    {
        case Localizer::CN: evLang = kEvLangCn; break;
        case Localizer::DE: evLang = kEvLangDe; break;
        case Localizer::ES: evLang = kEvLangEs; break;
        case Localizer::FR: evLang = kEvLangFr; break;
        case Localizer::IT: evLang = kEvLangIt; break;
        case Localizer::JP: evLang = kEvLangJp; break;
        case Localizer::KR: evLang = kEvLangKr; break;
        case Localizer::EN:
        default: evLang = kEvLangEn; break;
    }
    
    return evLang;
}

MenuDialogManager::MenuDialogManager(int category)
	:
Prop(category),
mBetaIQRating(0),
mBetaPuzzleName(""),
mQueryKey(kEvNull),
mMenuButton(NULL),
mBackDrop(NULL),
mBackDropTweener(NULL)
{

}

MenuDialogManager::~MenuDialogManager(void)
{
	mScene->deregisterForTouches(CMTouches::TC_DIALOGS, this);
    mScene->subscribeToInputUpdates(this, true);
    mScene->deregisterResDependent(this);
    mScene->deregisterLocalizable(this);
	mMenuDialogStack.clear();
	
	for (DlgMap::iterator it = mMenuDialogs.begin(); it != mMenuDialogs.end(); ++it)
		CC_SAFE_RELEASE(it->second);
	mMenuDialogs.clear();

	for (BtnMap::iterator it = mMenuButtons.begin(); it != mMenuButtons.end(); ++it)
		CC_SAFE_RELEASE(it->second);
	mMenuButtons.clear();

	if (mBackDropTweener)
	{
		mBackDropTweener->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mBackDropTweener);
	}
}

bool MenuDialogManager::init(void)
{
	CCPoint dialogPos = ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2);

	mBackDrop = CMUtils::createColoredQuad(CCSizeMake(mScene->getFullscreenWidth(), mScene->getFullscreenHeight()));
	mBackDrop->setPosition(dialogPos);
	mBackDrop->setColor(CMUtils::uint2color3B(0));
	mBackDrop->setOpacity(0);
	addChild(mBackDrop);

	mBackDropTweener = new ByteTweener(0, this, CMTransitions::EASE_OUT);

	// MenuEsc
	{
#if defined(CHEEKY_DESKTOP)
        int evCodes[] = { kEvResume, kEvOptions, kEvCredits, kEvGodMode, kEvExit };
#elif defined(__ANDROID__)
		int evCodes[] = { kEvResume, kEvOptions, kEvCredits, kEvGodMode, kEvExit };
#else
		int evCodes[] = { kEvResume, kEvOptions, kEvCredits, kEvGodMode };
#endif
		MenuDialog* dialog = MenuBuilder::buildMenuEscDialog(getCategory(), this, CMUtils::makeVector(evCodes));
		dialog->setPosition(dialogPos);
		addDialog("MenuEsc", dialog);
	}

	// PlayfieldEsc
	{
#if CM_BETA
		int evCodes[] = { kEvResume, kEvResetPuzzle, kEvOptions, kEvBetaReportBug, kEvMenu };
#else
        int evCodes[] = { kEvResume, kEvResetPuzzle, kEvOptions, kEvMenu };
#endif
		MenuDialog* dialog = MenuBuilder::buildPlayfieldEscDialog(getCategory(), this, CMUtils::makeVector(evCodes));
		dialog->setPosition(dialogPos);
		addDialog("PlayfieldEsc", dialog);
	}

	// Options
	{
#ifdef CHEEKY_MOBILE
		int evCodes[] = { kEvMusic, kEvSfx, kEvCloud, kEvColorBlind, kEvBack }; // Cloud filtered for Android in MenuBuilderIOS
#else
        int evCodes[] = { kEvMusic, kEvSfx, kEvDisplay, kEvLang, kEvCloud, kEvColorBlind, kEvBack };
#endif
		MenuDialog* dialog = MenuBuilder::buildOptionsDialog(getCategory(), this, CMUtils::makeVector(evCodes));
		dialog->setPosition(dialogPos);
		addDialog("Options", dialog);
	}
    
#ifdef CHEEKY_DESKTOP
    // Display
	{
		int evCodes[] = { kEvWindowed, kEvFullscreen, kEvBack };
		MenuDialog* dialog = MenuBuilder::buildDisplayDialog(getCategory(), this, CMUtils::makeVector(evCodes));
		dialog->setPosition(dialogPos);
		addDialog("Display", dialog);
	}
    
    // Language
    {
		int evCodes[] =
        {
            kEvLangEn, kEvLangCn, kEvLangDe, kEvLangEs,
            kEvLangFr, kEvLangIt, kEvLangJp, kEvLangKr,
            kEvBack
        };
		MenuDialog* dialog = MenuBuilder::buildLanguageDialog(getCategory(), this, CMUtils::makeVector(evCodes));
		dialog->setPosition(dialogPos);
		addDialog("Language", dialog);
	}
#endif
    
    // Credits
    {
        int evCode = kEvBack;
		MenuDialog* dialog = MenuBuilder::buildCreditsDialog(getCategory(), this, evCode);
		dialog->setPosition(dialogPos);
		addDialog("Credits", dialog);
    }

	// Query
	{
		int evCodes[] = { kEvYes, kEvNo };
		MenuDialog* dialog = MenuBuilder::buildQueryDialog(getCategory(), this, CMUtils::makeVector(evCodes));
		dialog->setPosition(dialogPos);
		addDialog("Query", dialog);
	}
    
#if CM_BETA
    // BetaReportBugThanks
    {
        MenuDialog* dialog = MenuBuilder::buildBetaReportBugThanksDialog(getCategory(), this, kEvBack);
		dialog->setPosition(dialogPos);
		addDialog("BetaReportBugThanks", dialog);
    }
    
    // BetaIQRating
//    {
//        MenuDialog* dialog = MenuBuilder::buildBetaIQRatingDialog(getCategory(), this, kEvBetaIQRatingSubmitted);
//		dialog->setPosition(dialogPos);
//		addDialog("BetaIQRating", dialog);
//    }
#endif

	refreshMusicText();
	refreshSfxText();

	mScene->registerForTouches(CMTouches::TC_DIALOGS, this);
    mScene->subscribeToInputUpdates(this, true);
    mScene->registerResDependent(this);
    mScene->registerLocalizable(this);
    
	setVisible(isTouchEnabled());
	return true;
}

MenuDialog* MenuDialogManager::getCurrentDialog(void) const
{
	return mMenuDialogStack.size() > 0 ? mMenuDialogStack[mMenuDialogStack.size()-1] : NULL;
}

int MenuDialogManager::getStackHeight(void) const
{
	return (int)mMenuDialogStack.size();
}

void MenuDialogManager::applyGameSettings(void)
{
    refreshMusicText();
    refreshSfxText();
    refreshMusicGauge();
    refreshSfxGauge();
    refreshCloudText();
    refreshColorBlindText();
    refreshLanguageText();
    refreshLowPowerModeText();
}

void MenuDialogManager::resolutionDidChange(void)
{
    if (mBackDrop)
    {
        mBackDrop->setTextureRect(CCRectMake(0, 0, mScene->getFullscreenWidth(), mScene->getFullscreenHeight()));
        mBackDrop->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
    }
    
    for (DlgMap::iterator it = mMenuDialogs.begin(); it != mMenuDialogs.end(); ++it)
        it->second->resolutionDidChange();
    
    refreshDisplayModeHighlight();
}

void MenuDialogManager::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    for (DlgMap::iterator it = mMenuDialogs.begin(); it != mMenuDialogs.end(); ++it)
        it->second->localeDidChange(fontKey, FXFontKey);
    
    for (BtnMap::iterator it = mMenuButtons.begin(); it != mMenuButtons.end(); ++it)
    {
        it->second->localeDidChange(fontKey, FXFontKey);
        positionMenuButton(it->second);
    }
    
    refreshLanguageText();
}

bool MenuDialogManager::isDialogOnStack(MenuDialog* dialog)
{
    if (dialog)
    {
        DlgStack::iterator findIt = std::find(mMenuDialogStack.begin(), mMenuDialogStack.end(), dialog);
        return findIt != mMenuDialogStack.end();
    }
    
    return false;
}

bool MenuDialogManager::isEscDialogShowing(void)
{
    return isDialogOnStack(getDialogForKey("MenuEsc")) || isDialogOnStack(getDialogForKey("PlayfieldEsc"));
}

bool MenuDialogManager::isExitDialogShowing(void)
{
	return isEscDialogShowing() && mQueryKey == kEvExit;
}

void MenuDialogManager::refreshMusicText(void)
{
#ifdef CHEEKY_MOBILE
    bool isMusicEnabled = mScene->isMusicEnabled();
	MenuDialog* dialog = getDialogForKey("Options");
	if (dialog)
		dialog->setMenuItemTextForTag(kEvMusic, isMusicEnabled ? "Music: On" : "Music: Off");
#endif
}

void MenuDialogManager::refreshSfxText(void)
{
#ifdef CHEEKY_MOBILE
	bool isSfxEnabled = mScene->isSfxEnabled();
	MenuDialog* dialog = getDialogForKey("Options");
	if (dialog)
		dialog->setMenuItemTextForTag(kEvSfx, isSfxEnabled ? "Sfx: On" : "Sfx: Off");
#endif
}

void MenuDialogManager::refreshMusicGauge(void)
{
#ifdef CHEEKY_DESKTOP
    MenuDialog* dialog = getDialogForKey("Options");
    if (dialog)
    {
        MenuItem* item = dialog->getMenuItem(kEvMusic);
        if (item)
        {
            GaugeMenuItem* gauge = static_cast<GaugeMenuItem*>(item);
            gauge->setGaugeLevel(mScene->getMusicVolume());
        }
    }
#endif
}

void MenuDialogManager::refreshSfxGauge(void)
{
#ifdef CHEEKY_DESKTOP
    MenuDialog* dialog = getDialogForKey("Options");
    if (dialog)
    {
        MenuItem* item = dialog->getMenuItem(kEvSfx);
        if (item)
        {
            GaugeMenuItem* gauge = static_cast<GaugeMenuItem*>(item);
            gauge->setGaugeLevel(mScene->getSfxVolume());
        }
    }
#endif
}

void MenuDialogManager::refreshDisplayModeHighlight(void)
{
#ifdef CHEEKY_DESKTOP
    MenuDialog* dialog = getDialogForKey("Display");
    if (dialog)
    {
        bool isFullscreen = mScene->isFullscreen();
        MenuItem* item = dialog->getMenuItem(kEvWindowed);
        if (item)
        {
            item->setSelectedFrameColor(!isFullscreen ? MenuBuilder::kMenuOrange : MenuBuilder::kMenuPressedColor);
            item->setEnabledLabelColor(!isFullscreen ? MenuBuilder::kMenuOrange : MenuBuilder::kMenuEnableColor);
            item->setSelectedLabelColor(!isFullscreen ? MenuBuilder::kMenuOrange : MenuBuilder::kMenuSelectedColor);
        }
        
        item = dialog->getMenuItem(kEvFullscreen);
        if (item)
        {
            item->setSelectedFrameColor(isFullscreen ? MenuBuilder::kMenuOrange : MenuBuilder::kMenuPressedColor);
            item->setEnabledLabelColor(isFullscreen ? MenuBuilder::kMenuOrange : MenuBuilder::kMenuEnableColor);
            item->setSelectedLabelColor(isFullscreen ? MenuBuilder::kMenuOrange : MenuBuilder::kMenuSelectedColor);
        }
    }
#endif
}

void MenuDialogManager::refreshCloudText(void)
{
#ifdef __APPLE__
    MenuDialog* dialog = getDialogForKey("Options");
	if (dialog)
		dialog->setMenuItemTextForTag(kEvCloud, mScene->getUserDefaultBool(CMSettings::B_CLOUD_ENABLED, true)
                                      ? MenuBuilder::kCloudEnabled
                                      : MenuBuilder::kCloudDisabled);
#endif
}

void MenuDialogManager::refreshColorBlindText(void)
{
    MenuDialog* dialog = getDialogForKey("Options");
	if (dialog)
		dialog->setMenuItemTextForTag(kEvColorBlind, mScene->getUserDefaultBool(CMSettings::B_COLOR_BLIND_MODE)
                                      ? MenuBuilder::kColorBlindModeOn
                                      : MenuBuilder::kColorBlindModeOff);
}

void MenuDialogManager::refreshLanguageText(void)
{
#ifdef CHEEKY_DESKTOP
    MenuDialog* dialog = getDialogForKey("Language");
	if (dialog)
    {
        int onTag = locale2EvLang(mScene->getLocale());
        for (int i = 0; i < kNumEvLangs; ++i)
        {
            MenuItem* menuItem = dialog->getMenuItem(kEvLangs[i]);
            if (menuItem)
            {
                if (kEvLangs[i] == onTag)
                {
                    menuItem->setEnabledLabelColor(MenuBuilder::kMenuOrange);
                    menuItem->setPressedLabelColor(MenuBuilder::kMenuOrange);
                    menuItem->setSelectedLabelColor(MenuBuilder::kMenuOrange);
                }
                else
                {
                    menuItem->setEnabledLabelColor(MenuBuilder::kMenuEnableColor);
                    menuItem->setPressedLabelColor(MenuBuilder::kMenuPressedColor);
                    menuItem->setSelectedLabelColor(MenuBuilder::kMenuPressedColor);
                }
            }
        }
    }
#endif
}

void MenuDialogManager::refreshLowPowerModeText(void)
{
    // TODO
}

void MenuDialogManager::addLevelUnlockedDialog(const char* key, int levelIndex)
{
	removeDialog(key);

	MenuDialog* dialog = MenuBuilder::buildLevelUnlockedDialog(getCategory(), levelIndex, this, kEvLevelUnlocked);
	dialog->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
	addDialog(key, dialog);
}

MenuDialog* MenuDialogManager::createCloudQueryDialog(void)
{
    int evCodes[] = { kEvYes, kEvNo };
    MenuDialog* dialog = MenuBuilder::buildCloudQueryDialog(getCategory(), this, CMUtils::makeVector(evCodes));
	dialog->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
	return dialog;
}

MenuDialog* MenuDialogManager::createLevelCompletedDialog(int levelIndex)
{
	MenuDialog* dialog = MenuBuilder::buildLevelCompletedDialog(getCategory(), levelIndex, this, kEvLevelCompleted);
	dialog->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
	return dialog;
}

void MenuDialogManager::showPuzzleWizardDialog(void)
{
    const char* key = "PuzzleWizard";
	if (getDialogForKey(key) == NULL)
	{
		MenuDialog* dialog = MenuBuilder::buildPuzzleWizardDialog(getCategory(), this, kEvPuzzleWizard);
		dialog->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
		addDialog(key, dialog);
	}
    
    pushDialogForKey(key, true);
}

void MenuDialogManager::addRateTheGameDialog(const char* key)
{
    CCAssert(false, "This dialog no longer supported due to font restrictions. Replaced with -Rate- button.");
    return;
    
//	if (getDialogForKey(key) == NULL)
//	{
//        int evCodes[] = { kEvRateYes, kEvRateNo };
//		MenuDialog* dialog = MenuBuilder::buildRateTheGameDialog(getCategory(), this, CMUtils::makeVector(evCodes));
//		dialog->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
//		addDialog(key, dialog);
//	}
}

void MenuDialogManager::addDialog(const char* key, MenuDialog* dialog)
{
	if (key == NULL || dialog == NULL)
		return;

	std::string stdKey(key);
	if (!stdKey.empty() && getDialogForKey(key) == NULL)
	{
		mMenuDialogs[stdKey] = dialog;
		dialog->retain();
	}
}

void MenuDialogManager::removeDialog(const char* key)
{
	MenuDialog* dialog = getDialogForKey(key);
	if (dialog)
	{
		mMenuDialogs.erase(key);
		if (dialog == getCurrentDialog())
			popDialog();
        
        CMUtils::erase(mMenuDialogStack, dialog);
		CC_SAFE_RELEASE_NULL(dialog);
	}
}

std::string MenuDialogManager::getKeyForDialog(MenuDialog* dialog)
{
    std::string key("");
    for (DlgMap::iterator it = mMenuDialogs.begin(); it != mMenuDialogs.end(); ++it)
    {
        if (it->second == dialog)
        {
            key = it->first;
            break;
        }
    }
    return key;
}

MenuDialog* MenuDialogManager::getDialogForKey(const char* key)
{
	MenuDialog* dialog = NULL;
	if (key)
	{
		std::string stdKey(key);
		if (!stdKey.empty())
		{
			DlgMap::iterator it = mMenuDialogs.find(stdKey);
			dialog = it != mMenuDialogs.end() ? (*it).second : NULL;
		}
	}
	return dialog;
}

void MenuDialogManager::pushDialogForKey(const char* key, bool animate)
{
    if (animate)
        pushDialogForKey(key, MenuDialog::DROP);
    else if (getStackHeight() == 0)
        pushDialogForKey(key, MenuDialog::ZOOM_IN);
    else
        pushDialogForKey(key, MenuDialog::ANIM_NONE);
}

void MenuDialogManager::pushDialogForKey(const char* key, MenuDialog::AnimState anim)
{
    if (key == NULL)
        return;
    
	MenuDialog* dialog = getDialogForKey(key), *topDialog = getCurrentDialog();
	if (dialog && !isDialogOnStack(dialog))
	{
		int insertAt = -1;
		for (int i = 0; i < (int)mMenuDialogStack.size(); ++i)
		{
			if (dialog->getPriority() >= mMenuDialogStack[i]->getPriority())
			{
				insertAt = i;
				break;
			}
		}
        
        BridgingUtility::setCrashContext(key, BridgingUtility::CRASH_CONTEXT_MENU);
        
		if (insertAt == -1)
			mMenuDialogStack.push_back(dialog);
		else
			mMenuDialogStack.insert(mMenuDialogStack.begin() + insertAt, dialog);
        
        dialog->resetNav();
        dialog->removeFromParent();
		addChild(dialog);
        
		if (getCurrentDialog() != topDialog)
		{
			if (topDialog)
				topDialog->hide();
			getCurrentDialog()->show(anim);
		}
		else
			dialog->hide();
        
		if (getStackHeight() == 1)
		{
            mScene->pushFocusState(CMInputs::FOCUS_STATE_MENU_DIALOG, true);
			setVisible(isTouchEnabled());
			updateMenuButton();
			mBackDropTweener->reset(mBackDrop->getOpacity(), kBackdropMaxOpacity,
                                    ((kBackdropMaxOpacity - mBackDrop->getOpacity()) / (float)kBackdropMaxOpacity) * kBackdropTweenerDuration);
            BridgingUtility::setStatusBarHidden(false);
		}
	}
}

void MenuDialogManager::pushCloudQueryDialog(void)
{
//    const char* key = "CloudQuery";
//    if (getDialogForKey(key) == NULL)
//	{
//		MenuDialog* dialog = createCloudQueryDialog();
//		addDialog(key, dialog);
//	}
//    
//    mQueryKey = kEvCloud;
//	pushDialogForKey(key);
}

void MenuDialogManager::pushLevelCompletedDialog(int levelIndex, bool animate)
{
	levelIndex = MAX(0, MIN(LevelMenu::kNumLevels - 1, levelIndex));
	std::string key = CMUtils::strConcatVal("LevelCompleted", levelIndex);
	if (getDialogForKey(key.c_str()) == NULL)
	{
		MenuDialog* dialog = createLevelCompletedDialog(levelIndex);
		addDialog(key.c_str(), dialog);
	}

	pushDialogForKey(key.c_str(), animate);
}

void MenuDialogManager::pushBetaIQRatingDialog(int IQValue, std::string puzzleName)
{
//    MenuDialog* dialog = getDialogForKey("BetaIQRating");
//    if (dialog && !puzzleName.empty())
//    {
//        mBetaPuzzleName = puzzleName;
//        
//        {
//            CCNode* item = dialog->getContentItem(kEvBetaIQRatingSubmitted);
//            if (item)
//            {
//                CCLabelBMFont* caption = static_cast<CCLabelBMFont*>(item);
//                std::string captionStr = CMUtils::strConcatVal("Please adjust the IQ rating for\nthis puzzle if you disagree with\nits present value of ", IQValue).append(".");
//                caption->setString(captionStr.c_str());
//            }
//            
//        }
//        
//        {
//            CCNode* item = dialog->getContentItem(IQSlider::EV_TYPE_IQ_CHANGED());
//            if (item)
//            {
//                IQSlider* slider = static_cast<IQSlider*>(item);
//                slider->setIQValue(IQValue);
//            }
//        }
//        
//        pushDialogForKey("BetaIQRating");
//    }
}

void MenuDialogManager::popDialog(void)
{
	if (getStackHeight() == 0)
		return;
    
    if (getStackHeight() == 1)
    {
        MenuDialog* currentDialog = getCurrentDialog();
        if (currentDialog)
            currentDialog->hide(MenuDialog::ZOOM_OUT); // Will remove itself from display list
        BridgingUtility::setStatusBarHidden(true);
    }
    else
    {
        if (getCurrentDialog())
            removeChild(getCurrentDialog());
    }

	mMenuDialogStack.pop_back();

    MenuDialog* currentDialog = getCurrentDialog();
	if (currentDialog)
    {
		currentDialog->show();
        std::string dialogKey = getKeyForDialog(currentDialog);
        if (!dialogKey.empty())
            BridgingUtility::setCrashContext(dialogKey.c_str(), BridgingUtility::CRASH_CONTEXT_MENU);
    }

	if (getStackHeight() == 0)
	{
		mQueryKey = kEvNull;

        if (getMenuButton())
            updateMenuButton();
        else
            mScene->popFocusState(CMInputs::FOCUS_STATE_MENU_DIALOG, true);
		mBackDropTweener->reset(mBackDrop->getOpacity(), 0, (mBackDrop->getOpacity() / (float)kBackdropMaxOpacity) * kBackdropTweenerDuration);
		mScene->flushUserDefaults();
        BridgingUtility::setCrashContext("None", BridgingUtility::CRASH_CONTEXT_MENU);
	}
}

void MenuDialogManager::popAllDialogs(void)
{
	while (mMenuDialogStack.size() > 0)
	{
		int count = (int)mMenuDialogStack.size();
		popDialog();
		if (count == (int)mMenuDialogStack.size()) // Avoid infinite loop in case pop fails.
			break;
	}

	setMenuButton(NULL);
}

void MenuDialogManager::addMenuButton(const char* key, FXButton* button)
{
	if (key && button)
	{
		removeMenuButton(key);
		mMenuButtons[key] = button;
		button->retain();
        positionMenuButton(button);
	}
}

void MenuDialogManager::removeMenuButton(const char* key)
{
	if (key)
	{
		BtnMap::iterator it = mMenuButtons.find(key);
		if (it != mMenuButtons.end())
		{
			if (it->second == getMenuButton())
				setMenuButton(NULL);
			mMenuButtons.erase(key);
		}
	}
}

void MenuDialogManager::showMenuButton(const char* key)
{
	if (key)
	{
		BtnMap::iterator it = mMenuButtons.find(key);
		if (it != mMenuButtons.end())
        {
			setMenuButton(it->second);
            
            if (getStackHeight() == 0)
                mScene->pushFocusState(CMInputs::FOCUS_STATE_MENU_DIALOG, true);
        }
	}
}

void MenuDialogManager::hideMenuButton(const char* key)
{
	if (key && getMenuButton())
	{
		BtnMap::iterator it = mMenuButtons.find(key);
		if (it != mMenuButtons.end() && it->second == getMenuButton())
			setMenuButton(NULL);
	}
}

void MenuDialogManager::setMenuButton(FXButton* value)
{
	if (value == mMenuButton)
		return;

	if (mMenuButton)
		mMenuButton->removeFromParent();
	
	mMenuButton = value;

	if (mMenuButton)
	{
		mMenuButton->setVisible(getStackHeight() == 0);
		addChild(mMenuButton);
	}
    else if (getStackHeight() == 0)
        mScene->popFocusState(CMInputs::FOCUS_STATE_MENU_DIALOG, true);
	
	setVisible(isTouchEnabled());
}

void MenuDialogManager::updateMenuButton(void)
{
	if (getMenuButton())
		getMenuButton()->setVisible(getStackHeight() == 0);
}

void MenuDialogManager::positionMenuButton(FXButton* button)
{
    PlayfieldController* scene = dynamic_cast<PlayfieldController*>(mScene);
	if (scene && scene->getPuzzleController() && button)
	{
		PuzzleBoard* board = scene->getPuzzleController()->getPuzzleBoard();
		if (board)
		{
			vec2 boardDims = board->getScaledBoardDimensions();
            CCSize buttonSize = button->getContentSize();
			CCPoint pos = ccp(
                              mScene->getViewWidth() / 2 + boardDims.x / 2 - buttonSize.width / 2,
                              mScene->getViewHeight() / 2 - (boardDims.y / 2 + 0.4f * buttonSize.height));
			button->setPosition(pos);
		}
	}
}

bool MenuDialogManager::isTouchEnabled(void) const
{
	return getStackHeight() > 0 || getMenuButton() != NULL;
}

void MenuDialogManager::onEvent(int evType, void* evData)
{
	//static int localeIndex = 0;
	if (evType == MenuDialog::EV_TYPE_ITEM_DID_ACTIVATE())
	{
		switch (*(int*)evData)
		{
			case kEvResume:
//			{
//				if (localeIndex >= kNumEvLangs)
//					localeIndex = 0;
//				mScene->setLocale(evLang2Locale(kEvLangs[localeIndex++]));
//			}
                
				mScene->hideEscDialog();
				break;
			case kEvOptions:
				pushDialogForKey("Options");
				break;
			case kEvResetPuzzle:
				{
                    popAllDialogs();
                    mScene->resetCurrentPuzzle();
				}
				break;
			case kEvMenu:
                popAllDialogs();
                mScene->abortCurrentPuzzle();
				mScene->returnToPuzzleMenu();
				break;
			case kEvMusic:
            {
#ifdef CHEEKY_MOBILE
				mScene->enableMusic(!mScene->isMusicEnabled());
				refreshMusicText();
#else
                MenuDialog* dialog = getDialogForKey("Options");
                if (dialog)
                {
                    MenuItem* item = dialog->getMenuItem(kEvMusic);
                    if (item)
                    {
                        GaugeMenuItem* gauge = static_cast<GaugeMenuItem*>(item);
                        mScene->setMusicVolume(gauge->getGaugeLevel());
                    }
                }
#endif
            }
				break;
			case kEvSfx:
            {
#ifdef CHEEKY_MOBILE
				mScene->enableSfx(!mScene->isSfxEnabled());
				refreshSfxText();
#else
                MenuDialog* dialog = getDialogForKey("Options");
                if (dialog)
                {
                    MenuItem* item = dialog->getMenuItem(kEvSfx);
                    if (item)
                    {
                        GaugeMenuItem* gauge = static_cast<GaugeMenuItem*>(item);
                        mScene->setSfxVolume(gauge->getGaugeLevel());
                    }
                }
#endif
            }
				break;
            case kEvDisplay:
                refreshDisplayModeHighlight();
                pushDialogForKey("Display");
                break;
            case kEvColorBlind:
                mScene->setColorScheme(mScene->getColorScheme() == Puzzles::CS_NORMAL
                                       ? Puzzles::CS_COLOR_BLIND
                                       : Puzzles::CS_NORMAL);
                break;
            case kEvWindowed:
                if (mScene->isFullscreen())
                {
                    mScene->setFullscreen(false);
                    refreshDisplayModeHighlight();
                }
                break;
            case kEvFullscreen:
                if (!mScene->isFullscreen())
                {
                    mScene->setFullscreen(true);
                    refreshDisplayModeHighlight();
                }
                break;
            case kEvCredits:
                pushDialogForKey("Credits");
                break;
			case kEvBack:
				popDialog();
				break;
			case kEvLevelUnlocked:
				if (getCurrentDialog() && !getCurrentDialog()->isAnimating())
                {
					popDialog();
#if CM_BETA
                    //mScene->showBetaIQRatingDialog();
#endif
                }
				break;
			case kEvPuzzleWizard:
				if (getCurrentDialog() && !getCurrentDialog()->isAnimating())
                {
					popDialog();
                    removeDialog("PuzzleWizard");
#if CM_BETA
                    //mScene->showBetaIQRatingDialog();
#endif
                }
				break;
			case kEvLevelCompleted:
                if (getCurrentDialog() && !getCurrentDialog()->isAnimating())
                {
					popDialog();
                    dispatchEvent(EV_TYPE_DID_CLOSE_LEVEL_COMPLETED_DIALOG());
                }
				break;
			case kEvYes:
                switch (mQueryKey)
                {
                    case kEvExit:
#if defined(CHEEKY_DESKTOP) || defined(__ANDROID__)
                        mScene->exitApp();
#endif
                        break;
                    case kEvNull:
                    default:
                        popDialog();
                        break;
                }
                
                mQueryKey = kEvNull;
				break;
			case kEvNo:
                switch (mQueryKey)
                {
                    default:
                        break;
                }
                
                mQueryKey = kEvNull;
                popDialog();
				break;
            case kEvRateYes:
                {
                    mScene->userRespondedToRatingPrompt(true);
                    popDialog();
                }
                break;
            case kEvRateNo:
                {
                    mScene->userRespondedToRatingPrompt(false);
                    popDialog();
                }
                break;
			case kEvGodMode:
				{
					MenuDialog* dialog = getCurrentDialog();
					if (dialog && dialog == getDialogForKey("MenuEsc"))
					{
						const char* itemText = dialog->getMenuItemTextForTag(kEvGodMode);
						if (itemText)
						{
							if (strcmp(itemText, MenuBuilder::kGodModeOff) == 0)
							{
								dialog->setMenuItemTextForTag(kEvGodMode, MenuBuilder::kGodModeOn);
								mScene->enableGodMode(true);
							}
							else
							{
								dialog->setMenuItemTextForTag(kEvGodMode, MenuBuilder::kGodModeOff);
								mScene->enableGodMode(false);
							}
						}
					}
				}
				break;
            case kEvCloud:
                {
                    bool newCloudState = !mScene->getUserDefaultBool(CMSettings::B_CLOUD_ENABLED, true);
                    mScene->setUserDefault(CMSettings::B_CLOUD_ENABLED, newCloudState);
                    mScene->flushUserDefaults();
                    refreshCloudText();
                    
                    if (newCloudState)
                        mScene->userEnabledCloud();
                    else
                        mScene->enableCloud(newCloudState);
                }
                break;
            case kEvExit:
                mQueryKey = kEvExit;
                pushDialogForKey("Query");
                break;
            case kEvBetaReportBug:
                mScene->reportBuggedPuzzle();
                break;
            case kEvBetaIQRatingSubmitted:
            {
#if CM_BETA
                std::string IQRecommendation = CMUtils::strConcatVal("", mBetaIQRating);
                IQRecommendation += std::string("_[") + mScene->getGameModeSuffix() + "]";
                const char* params[] = {
                    "IQ", IQRecommendation.c_str()
                };
                
                std::string evName(BridgingUtility::EV_NAME_BETA_SUGGEST_IQ_);
                evName += mBetaPuzzleName;
                BridgingUtility::logEvent(evName.c_str(), CMUtils::makeStrMap(params));
                TFManager::TFM()->applyPendingState();
#endif
                popDialog();
            }
                break;
            case kEvPrivacy:
                BridgingUtility::openURL("http://www.cheekymammoth.com/privacy.html");
                break;
            case kEvLowPowerMode:
            {
                bool newLowPowerModeState = !mScene->getUserDefaultBool(CMSettings::B_LOW_POWER_MODE_ENABLED);
                mScene->enableLowPowerMode(newLowPowerModeState);
            }
                break;
            case kEvLang:
                pushDialogForKey("Language");
                break;
            case kEvLangEn:
            case kEvLangCn:
            case kEvLangDe:
            case kEvLangEs:
            case kEvLangFr:
            case kEvLangIt:
            case kEvLangJp:
            case kEvLangKr:
            {
                Localizer::LocaleType locale = evLang2Locale(*(int*)evData);
                if (locale != mScene->getLocale())
                {
#ifdef CHEEKY_DESKTOP
                    // Set the system locale so that the manual in-game override is respected on next launch. Without this, the system locale
                    // will override their manual setting in order to respect the system locale change ahead of all else. But if there
                    // has been no system locale change since last launch, then we respect the manual in-game setting.
                    std::string currentSysLocaleStr = Localizer::getLocaleStringFromCurrentUICultureExplicit();
                    mScene->setUserDefault(CMSettings::S_SYSLOCALE, currentSysLocaleStr.c_str());
#endif
                    mScene->setLocale(locale);
                }
            }
                break;
		}
	}
	else if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN()
		|| evType == TouchPad::EV_TYPE_TOUCH_ENDED()
		|| evType == TouchPad::EV_TYPE_TOUCH_CANCELLED()
		|| evType == TouchPad::EV_TYPE_TOUCH_MOVED())
	{
		if (isTouchEnabled())
		{
			CMTouches::TouchNotice* touchNotice = (CMTouches::TouchNotice*)evData;
			if (touchNotice)
			{
				if (getStackHeight() > 0) // MenuDialogs
				{
					touchNotice->retainFocus(this);

					MenuDialog* dialog = getCurrentDialog();
					if (dialog)
						dialog->onEvent(evType, evData);
				}
				else // MenuButtons
				{
					FXButton* button = getMenuButton();
					if (button)
					{
                        CCPoint pos = this->convertToNodeSpace(touchNotice->pos);
                        CCRect buttonBounds = CMUtils::boundsInSpace(this, button);
                        if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN())
                        {
                            if (buttonBounds.containsPoint(pos))
                            {
                                touchNotice->retainFocus(this);
                                button->depress();
                            }
                        }
						else if (evType == TouchPad::EV_TYPE_TOUCH_ENDED() || evType == TouchPad::EV_TYPE_TOUCH_CANCELLED())
                            button->raise();
						else if (evType == TouchPad::EV_TYPE_TOUCH_MOVED())
                        {
                            CCRect cancelBounds = buttonBounds;
                            cancelBounds.setRect(
                                                 cancelBounds.origin.x - kTouchPadding,
                                                 cancelBounds.origin.y - kTouchPadding,
                                                 cancelBounds.size.width + 2 * kTouchPadding,
                                                 cancelBounds.size.height + 2 * kTouchPadding);
                            
                            if (!cancelBounds.containsPoint(pos))
                                button->reset();
                        }
					}
				}
			}
		}
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		if (mBackDrop && mBackDropTweener)
			mBackDrop->setOpacity(mBackDropTweener->getTweenedValue());
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
		if (getStackHeight() == 0)
			setVisible(isTouchEnabled());
	}
    else if (evType == IQSlider::EV_TYPE_IQ_CHANGED())
    {
#if CM_BETA
        if (evData)
        {
            IQSlider* slider = static_cast<IQSlider*>(evData);
            mBetaIQRating = slider->getIQIntervalValue();
            //CCLog("BetaIQRating changed: %d", mBetaIQRating);
        }
#endif
    }
}

void MenuDialogManager::willLoseFocus(void)
{
    popAllDialogs();
}

void MenuDialogManager::update(int controllerState)
{
#ifndef __ANDROID__
    ControlsManager* cm = ControlsManager::CM();
    MenuDialog* currentDialog = getCurrentDialog();
    if (currentDialog)
    {
        if (currentDialog->canGoBack())
        {
            if (cm->didCancel(true))
            {
                if (currentDialog == getDialogForKey("MenuEsc") || currentDialog == getDialogForKey("PlayfieldEsc"))
                    mScene->hideEscDialog();
                else
                    popDialog();
                return;
            }
        }
        else
        {
            if (cm->didKeyDepress(kVK_Escape))
            {
                mScene->showEscDialog();
                return;
            }
        }

        currentDialog->update(controllerState);
    }
    else
    {
        if (cm->didKeyDepress(kVK_Escape))
            mScene->showEscDialog();
        else
        {
            FXButton* menuButton = getMenuButton();
            if (menuButton && menuButton->isVisible())
            {
                if (cm->didAccept(true))
                    menuButton->depress();
                else if (cm->didAccept(false))
                    menuButton->raise();
            }
        }
    }
#endif
}

void MenuDialogManager::advanceTime(float dt)
{
	if (mBackDropTweener)
		mBackDropTweener->advanceTime(dt);

	MenuDialog* dialog = getCurrentDialog();
	if (dialog)
		dialog->advanceTime(dt);
}


