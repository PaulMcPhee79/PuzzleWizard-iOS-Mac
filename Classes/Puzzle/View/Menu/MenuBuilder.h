#ifndef __MENU_BUILDER_H__
#define __MENU_BUILDER_H__

#include "cocos2d.h"
#include <SceneControllers/SceneController.h>
class MenuDialog;
class MenuItem;
USING_NS_CC;

class MenuBuilder
{
public:
    static const uint kMenuEnableColor = 0xffff4d; // Yellow
    static const uint kMenuDarkYellow = 0xfccd36;
    static const uint kMenuOrange = 0xff8800;
    
#ifdef CHEEKY_MOBILE
    static const uint kMenuPressedColor = 0x8d3beb; // Purple
    static const uint kMenuSelectedColor = 0x8d3beb; // Purple
#else
    static const uint kMenuPressedColor = 0x22ff3e; // Green
    static const uint kMenuSelectedColor = 0x22ff3e; // Green
#endif
    static const uint kMenuBuilderDisabled = 0x8e8e8e; // Grey
    static const uint kMenuBuilderSlateBlue = 0x6a5acd;
    static const int kMenuFontSize = 44;
    
	static const char* kGodModeOn;
	static const char* kGodModeOff;
    
    static const char* kCloudEnabled;
	static const char* kCloudDisabled;
    
    static const char* kColorBlindModeOn;
	static const char* kColorBlindModeOff;

	static MenuDialog* buildMenuEscDialog(int category, IEventListener* listener, const std::vector<int>& evCodes);
	static MenuDialog* buildPlayfieldEscDialog(int category, IEventListener* listener, const std::vector<int>& evCodes);
	static MenuDialog* buildOptionsDialog(int category, IEventListener* listener, const std::vector<int>& evCodes);
    static MenuDialog* buildLanguageDialog(int category, IEventListener* listener, const std::vector<int>& evCodes);
    static MenuDialog* buildDisplayDialog(int category, IEventListener* listener, const std::vector<int>& evCodes);
    static MenuDialog* buildCreditsDialog(int category, IEventListener* listener, int evCode);
	static MenuDialog* buildQueryDialog(int category, IEventListener* listener, const std::vector<int>& evCodes);
    static MenuDialog* buildCloudQueryDialog(int category, IEventListener* listener, const std::vector<int>& evCodes);
	static MenuDialog* buildLevelUnlockedDialog(int category, int levelIndex, IEventListener* listener, int evCode);
	static MenuDialog* buildLevelCompletedDialog(int category, int levelIndex, IEventListener* listener, int evCode);
	static MenuDialog* buildPuzzleWizardDialog(int category, IEventListener* listener, int evCode);
    static MenuDialog* buildRateTheGameDialog(int category, IEventListener* listener, const std::vector<int>& evCodes);
    static MenuDialog* buildBetaReportBugThanksDialog(int category, IEventListener* listener, int evCode);
    static MenuDialog* buildBetaIQRatingDialog(int category, IEventListener* listener, int evCode);
    
	static SceneController* getScene(void);
	static void setScene(SceneController* value);

private:
	MenuBuilder(void);
	virtual ~MenuBuilder(void);
};
#endif // __MENU_BUILDER_H__
