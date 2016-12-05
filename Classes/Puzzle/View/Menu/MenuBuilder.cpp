
#include "MenuBuilder.h"
#include <Puzzle/View/Menu/MenuDialog.h>
#include <Puzzle/View/Menu/MenuCustomDialog.h>
#include <Puzzle/View/Menu/LevelIcon.h>
#include <Puzzle/View/Menu/LevelMenu.h>
#include <SceneControllers/PlayfieldController.h>
#include <Puzzle/Controllers/PuzzleController.h>
#include <Puzzle/View/Playfield/PuzzleBoard.h>
#include <Managers/InputManager.h>
#include <Utils/Utils.h>
#include <Utils/CMTypes.h>

#ifdef CHEEKY_MOBILE
#include "MenuBuilderIOS.h"
typedef MenuBuilderIOS MENU_BUILDER_CLASS;
#else
#include "MenuBuilderOSX.h"
typedef MenuBuilderOSX MENU_BUILDER_CLASS;
#endif

USING_NS_CC;

const char* MenuBuilder::kGodModeOn = "-DEV-  Unlock All : On ";
const char* MenuBuilder::kGodModeOff = "-DEV-  Unlock All : Off";

const char* MenuBuilder::kCloudEnabled  = "iCloud: Enabled";
const char* MenuBuilder::kCloudDisabled = "iCloud: Disabled";

const char* MenuBuilder::kColorBlindModeOn =  "Color Blind Mode: On";
const char* MenuBuilder::kColorBlindModeOff = "Color Blind Mode: Off";

SceneController* MenuBuilder::getScene(void)
{
    return MENU_BUILDER_CLASS::getScene();
}

void MenuBuilder::setScene(SceneController* value)
{
    MENU_BUILDER_CLASS::setScene(value);
}

MenuDialog* MenuBuilder::buildMenuEscDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    return MENU_BUILDER_CLASS::buildMenuEscDialog(category, listener, evCodes);
}

MenuDialog* MenuBuilder::buildPlayfieldEscDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    return MENU_BUILDER_CLASS::buildPlayfieldEscDialog(category, listener, evCodes);
}

MenuDialog* MenuBuilder::buildOptionsDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    return MENU_BUILDER_CLASS::buildOptionsDialog(category, listener, evCodes);
}

MenuDialog* MenuBuilder::buildLanguageDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    return MENU_BUILDER_CLASS::buildLanguageDialog(category, listener, evCodes);
}

MenuDialog* MenuBuilder::buildDisplayDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    return MENU_BUILDER_CLASS::buildDisplayDialog(category, listener, evCodes);
}

MenuDialog* MenuBuilder::buildCreditsDialog(int category, IEventListener* listener, int evCode)
{
    return MENU_BUILDER_CLASS::buildCreditsDialog(category, listener, evCode);
}

MenuDialog* MenuBuilder::buildQueryDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    return MENU_BUILDER_CLASS::buildQueryDialog(category, listener, evCodes);
}

MenuDialog* MenuBuilder::buildCloudQueryDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    return MENU_BUILDER_CLASS::buildCloudQueryDialog(category, listener, evCodes);
}

MenuDialog* MenuBuilder::buildLevelUnlockedDialog(int category, int levelIndex, IEventListener* listener, int evCode)
{
    return MENU_BUILDER_CLASS::buildLevelUnlockedDialog(category, levelIndex, listener, evCode);
}

MenuDialog* MenuBuilder::buildLevelCompletedDialog(int category, int levelIndex, IEventListener* listener, int evCode)
{
    return MENU_BUILDER_CLASS::buildLevelCompletedDialog(category, levelIndex, listener, evCode);
}

MenuDialog* MenuBuilder::buildPuzzleWizardDialog(int category, IEventListener* listener, int evCode)
{
    return MENU_BUILDER_CLASS::buildPuzzleWizardDialog(category, listener, evCode);
}

MenuDialog* MenuBuilder::buildRateTheGameDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    return MENU_BUILDER_CLASS::buildRateTheGameDialog(category, listener, evCodes);
}

MenuDialog* MenuBuilder::buildBetaReportBugThanksDialog(int category, IEventListener* listener, int evCode)
{
    return MENU_BUILDER_CLASS::buildBetaReportBugThanksDialog(category, listener, evCode);
}

MenuDialog* MenuBuilder::buildBetaIQRatingDialog(int category, IEventListener* listener, int evCode)
{
    return MENU_BUILDER_CLASS::buildBetaIQRatingDialog(category, listener, evCode);
}
