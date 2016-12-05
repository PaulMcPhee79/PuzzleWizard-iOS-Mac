#ifndef __MENU_BUILDER_IOS_H__
#define __MENU_BUILDER_IOS_H__

#include "cocos2d.h"
#include <SceneControllers/SceneController.h>
class MenuDialog;
class MenuItem;
USING_NS_CC;

class MenuBuilderIOS
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
    static const int kIOSDefaultMenuFontSize = 48;
    
	static SceneController* s_Scene;
    static float dialogScaleFactor(void);
	static MenuItem* createMenuItem(const char* text, int tag, int fontSize = kIOSDefaultMenuFontSize);
    static CCNode* createDialogBg(float width, float height);
    
	MenuBuilderIOS(void);
	virtual ~MenuBuilderIOS(void);
};
#endif // __MENU_BUILDER_IOS_H__
