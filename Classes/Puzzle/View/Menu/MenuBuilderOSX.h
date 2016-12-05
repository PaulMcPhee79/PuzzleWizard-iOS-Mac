#ifndef __MENU_BUILDER_OSX_H__
#define __MENU_BUILDER_OSX_H__

#include "cocos2d.h"
#include <SceneControllers/SceneController.h>
class MenuDialog;
class MenuItem;
class GaugeMenuItem;
struct GaugeMenuStruct;
USING_NS_CC;

class MenuBuilderOSX
{
public:
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
    
	static SceneController* getScene(void) { return s_Scene; }
	static void setScene(SceneController* value) { s_Scene = value; }
    
private:
    static const int kOSXDefaultMenuFontSize = 40;
    
	static SceneController* s_Scene;
    static float scaleForFontSize(int fontSize);
    static MenuItem* createMenuItem(CCSpriteFrame* upFrame, CCSpriteFrame* downFrame, int tag);
	static MenuItem* createMenuItem(const char* text, int tag, int fontSize = kOSXDefaultMenuFontSize);
    static GaugeMenuItem* createGaugeMenuItem(GaugeMenuStruct* initData, int tag);
    static CCNode* createDialogBg(float width, float height);
	MenuBuilderOSX(void);
	virtual ~MenuBuilderOSX(void);
};
#endif // __MENU_BUILDER_OSX_H__
