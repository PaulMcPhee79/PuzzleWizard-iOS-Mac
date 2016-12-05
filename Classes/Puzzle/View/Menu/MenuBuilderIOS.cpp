
#include "MenuBuilderIOS.h"
#include "MenuBuilder.h"
#include <Puzzle/View/Menu/MenuDialog.h>
#include <Puzzle/View/Menu/MenuCustomDialog.h>
#include <Puzzle/View/Menu/LevelIcon.h>
#include <Puzzle/View/Menu/LevelMenu.h>
#include <SceneControllers/PlayfieldController.h>
#include <Puzzle/Controllers/PuzzleController.h>
#include <Puzzle/View/Playfield/PuzzleBoard.h>
#include <Prop/UIControls/IQSlider.h>
#include <Managers/InputManager.h>
#include <Utils/Utils.h>
#include <Utils/CMTypes.h>
#include <CCExtensions/CMScale9Sprite.h>
//USING_NS_CC;
USING_NS_CC_EXT;

static const float kMenuVertSeparation = 72.0f;
static const float kDefaultVertPadding = 20.0f;
static const float kCreditsScrollHeightFactor = 0.715f;

static const int kUIDPlayfieldEsc = 1000;
static const int kUIDCloudQuery = 1001;
static const int kUIDCredits = 1002;

SceneController* MenuBuilderIOS::s_Scene = NULL;

static float s_AspectRatio = -1;
float MenuBuilderIOS::dialogScaleFactor(void)
{
    if (s_AspectRatio < 0)
        s_AspectRatio = s_Scene->getViewportAspectRatio();
    return MAX(1.15f, MIN(1.4f, 1.4f - (1.5f - s_AspectRatio)));
}

MenuItem* MenuBuilderIOS::createMenuItem(const char* text, int tag, int fontSize)
{
	MenuItem* item = MenuItem::create(text, fontSize);
	item->setTag(tag);
	item->setEnabledLabelColor(MenuBuilder::kMenuEnableColor);
	item->setPressedLabelColor(MenuBuilder::kMenuPressedColor);
    item->setSelectedLabelColor(MenuBuilder::kMenuEnableColor);
    item->setSelectedFrameColor(MenuBuilder::kMenuEnableColor);
	return item;
}

CCNode* MenuBuilderIOS::createDialogBg(float width, float height)
{
    CCTexture2D* tex = getScene()->textureByName("dialog-bg9");
    CCRect rect = CCRectZero;
    rect.size = tex->getContentSize();
    
    CCSpriteFrame* spriteFrame = CCSpriteFrame::createWithTexture(tex, rect);
    CMScale9Sprite* sprite = CMScale9Sprite::createWithSpriteFrame(spriteFrame, CCRectMake(32, 86, 64, 22));
    sprite->setContentSize(CCSizeMake(width, height));
    sprite->setVisible(false);
    return sprite;
}

//enum LocaleType { EN = 0, CN, DE, ES, FR, IT, JP, KR, INVALID_LOCALE };
void customLayoutFunctionIOS(int uid, const std::vector<CCNode*>& layoutItems)
{
    switch (uid)
    {
        case kUIDPlayfieldEsc:
        {
            float width = 420;
            if (layoutItems.size() >= 1)
            {
                switch (Localizer::getLocale()) {
                    case Localizer::DE:
                        width = 500;
                        break;
                    case Localizer::FR:
                        width = 480;
                        break;
                    case Localizer::JP:
                        width = 480;
                        break;
                    default:
                        break;
                }
                
                layoutItems[0]->setContentSize(CCSizeMake(width, layoutItems[0]->getContentSize().height));
            }
        }
            break;
        case kUIDCloudQuery:
        {
            if (layoutItems.size() >= 3)
            {
                float width = 600;
                switch (Localizer::getLocale())
                {
                    case Localizer::CN:
                        width = 660;
                        break;
                    case Localizer::DE:
                        width = 660;
                        break;
                    case Localizer::ES:
                        width = 660;
                        break;
                    case Localizer::FR:
                        width = 720;
                        break;
                    case Localizer::IT:
                        width = 660;
                        break;
                    case Localizer::JP:
                        width = 780;
                        break;
                    default:
                        break;
                }
                
                layoutItems[0]->setContentSize(CCSizeMake(width, layoutItems[0]->getContentSize().height));
                
                CCSize bgSize = layoutItems[0]->getContentSize();
                layoutItems[1]->setPosition(ccp(-0.2f * bgSize.width, layoutItems[1]->getPositionY()));
                layoutItems[2]->setPosition(ccp(0.2f * bgSize.width, layoutItems[2]->getPositionY()));
            }
        }
            break;
        case kUIDCredits:
        {
            float width = 640;
            if (layoutItems.size() >= 3)
            {
                switch (Localizer::getLocale()) {
                    case Localizer::DE:
                        width = 760;
                        break;
                    case Localizer::ES:
                        width = 680;
                        break;
                    case Localizer::FR:
                        width = 700;
                        break;
                    case Localizer::IT:
                        width = 820;
                        break;
                    case Localizer::JP:
                        width = 720;
                        break;
                    case Localizer::KR:
                        width = 700;
                        break;
                    default:
                        break;
                }
                
                int itemIndex = 0;
                MenuCustomDialog* dialog = static_cast<MenuCustomDialog*>(layoutItems[itemIndex++]);
                
                CCNode* bg = layoutItems[itemIndex++];
                bg->setContentSize(CCSizeMake(width, bg->getContentSize().height));
                
                CCRect bgBounds = bg->boundingBox();
                dialog->setScrollBounds(CCRectMake(
                                                   bgBounds.origin.x,
                                                   bgBounds.origin.y + 0.25f * bgBounds.size.height,
                                                   bgBounds.size.width,
                                                   kCreditsScrollHeightFactor * bgBounds.size.height));
                
                CCNode* wizardProp = layoutItems[itemIndex];
                wizardProp->setPosition(ccp(
                                            bgBounds.size.width / 2 - (wizardProp->boundingBox().size.width / 2 + 30),
                                            -wizardProp->boundingBox().size.height / 3));
            }
        }
            break;
        default:
            break;
    }
}

MenuDialog* MenuBuilderIOS::buildMenuEscDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{

#ifdef __APPLE__
	// kEvResume, kEvOptions, kEvCredits, kGodMode
	CCAssert(evCodes.size() == 4, "MenuBuilder::buildMenuEscDialog - bad args.");
#else
	// kEvResume, kEvOptions, kEvCredits, kGodMode, kEvExit
		CCAssert(evCodes.size() == 5, "MenuBuilder::buildMenuEscDialog - bad args.");
#endif
    
	MenuDialog* dialog = MenuDialog::create(category, 30, CMInputs::HAS_FOCUS_MENU_DIALOG, NAV_VERT, listener);
    dialog->setCanGoBack(true);
    dialog->setRepeats(true);

#ifdef __APPLE__
	#if GOD_MODE
    	CCNode* bg = createDialogBg(540.0f, 350.0f);
	#else
    	CCNode* bg = createDialogBg(380.0f, 280.0f);
	#endif
#else
	#if GOD_MODE
    	CCNode* bg = createDialogBg(540.0f, 420.0f);
	#else
    	CCNode* bg = createDialogBg(380.0f, 350.0f);
	#endif
#endif

    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    
    int evIndex = 0;
	MenuItem* resume = MenuBuilderIOS::createMenuItem("Resume", evCodes[evIndex++]);
	resume->setPosition(ccp(0, bgBounds.size.height / 2 - 56));
	dialog->addMenuItem(resume);
    
    MenuItem* options = MenuBuilderIOS::createMenuItem("Options", evCodes[evIndex++]);
	options->setPosition(ccp(0, resume->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(options);
    
    MenuItem* credits = MenuBuilderIOS::createMenuItem("Credits", evCodes[evIndex++]);
	credits->setPosition(ccp(0, options->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(credits);
    
//    MenuItem* privacy = MenuBuilderIOS::createMenuItem("Privacy", evCodes[evIndex++]);
//	privacy->setPosition(ccp(0, credits->getPositionY() - kMenuVertSeparation));
//	dialog->addMenuItem(privacy);

#if GOD_MODE
	MenuItem* unlockAll = MenuBuilderIOS::createMenuItem(MenuBuilder::kGodModeOff, evCodes[evIndex++]);
	unlockAll->setPosition(ccp(0, credits->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(unlockAll);
#elif defined(__ANDROID__)
    ++evIndex;
    MenuItem* placeholder = credits;
#endif

#ifdef __ANDROID__
    MenuItem* exit = MenuBuilderIOS::createMenuItem("Exit Game", evCodes[evIndex]);
	exit->setPosition(ccp(0, placeholder->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(exit);
#endif
    
    dialog->setShowScale(dialogScaleFactor());
	return dialog;
}

MenuDialog* MenuBuilderIOS::buildPlayfieldEscDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
	// kEvResume, kEvResetPuzzle, kEvOptions, [kEvBetaReportBug], kEvMenu
#if CM_BETA
	CCAssert(evCodes.size() == 5, "MenuBuilder::buildPlayfieldEscDialog - bad args.");
#else
    CCAssert(evCodes.size() == 4, "MenuBuilder::buildPlayfieldEscDialog - bad args.");
#endif
	MenuDialog* dialog = MenuDialog::create(category, 30, CMInputs::HAS_FOCUS_MENU_DIALOG, NAV_VERT, listener);
	dialog->setCanGoBack(true);
    dialog->setRepeats(true);
	
#if CM_BETA
    CCNode* bg = createDialogBg(560.0f, 432.0f);
#else
    CCNode* bg = createDialogBg(420.0f, 348.0f);
#endif
    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    
    int evIndex = 0;
	MenuItem* resume = MenuBuilderIOS::createMenuItem("Resume", evCodes[evIndex++]);
	resume->setPosition(ccp(0, bgBounds.size.height / 2 - 56));
	dialog->addMenuItem(resume);
    
	MenuItem* reset = MenuBuilderIOS::createMenuItem("Reset Puzzle", evCodes[evIndex++]);
	reset->setPosition(ccp(0, resume->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(reset);
    
	MenuItem* options = MenuBuilderIOS::createMenuItem("Options", evCodes[evIndex++]);
	options->setPosition(ccp(0, reset->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(options);
    
    MenuItem* placeholder = options;
#if CM_BETA
    MenuItem* report = MenuBuilderIOS::createMenuItem("Report Broken Puzzle", evCodes[evIndex++]);
	report->setPosition(ccp(0, options->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(report);
    placeholder = report;
#endif
    
	MenuItem* menu = MenuBuilderIOS::createMenuItem("Puzzle Menu", evCodes[evIndex]);
	menu->setPosition(ccp(0, placeholder->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(menu);
    
#if !CM_BETA
    dialog->enableCustomLayout(kUIDPlayfieldEsc, customLayoutFunctionIOS);
    dialog->addCustomLayoutItem(bg);
    dialog->layoutCustomItems();
#endif
    
    dialog->setShowScale(dialogScaleFactor());
	return dialog;
}

MenuDialog* MenuBuilderIOS::buildOptionsDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    bool isCloudSupported = getScene()->isCloudSupported();
    
    // kEvMusic, kEvSfx, kEvCloud, kEvColorBlind, kEvBack
	CCAssert(evCodes.size() == 5, "MenuBuilder::buildOptionsDialog - bad args.");
	MenuDialog* dialog = MenuDialog::create(category, 20, CMInputs::HAS_FOCUS_MENU_DIALOG, NAV_VERT, listener);
	dialog->setCanGoBack(true);
    dialog->setRepeats(true);
    
    CCNode* bg = createDialogBg(580.0f, isCloudSupported ? 410.0f : 336.0f);
    dialog->addBgItem(bg);
    
    int evIndex = 0;
	CCRect bgBounds = bg->boundingBox();
    
	MenuItem* music = MenuBuilderIOS::createMenuItem("Music: On", evCodes[evIndex++]);
	music->setPosition(ccp(0, bgBounds.size.height / 2 - 60));
	dialog->addMenuItem(music);
    
	MenuItem* sfx = MenuBuilderIOS::createMenuItem("Sfx: On", evCodes[evIndex++]);
	sfx->setPosition(ccp(0, music->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(sfx);
    
    MenuItem* cloud = NULL;
    if (isCloudSupported)
    {
        cloud = MenuBuilderIOS::createMenuItem(MenuBuilder::kCloudDisabled, evCodes[evIndex++]);
        cloud->setPosition(ccp(0, sfx->getPositionY() - kMenuVertSeparation));
        dialog->addMenuItem(cloud);
    }
    else
    {
        ++evIndex;
        cloud = sfx;
    }
    
    MenuItem* colorBlind = MenuBuilderIOS::createMenuItem(MenuBuilder::kColorBlindModeOff, evCodes[evIndex++]);
	colorBlind->setPosition(ccp(0, cloud->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(colorBlind);
    
	MenuItem* back = MenuBuilderIOS::createMenuItem("back", evCodes[evIndex]);
	back->setPosition(ccp(0, colorBlind->getPositionY() - kMenuVertSeparation));
	dialog->addMenuItem(back);
    
    dialog->setShowScale(dialogScaleFactor());
	return dialog;
}

MenuDialog* MenuBuilderIOS::buildLanguageDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    CCAssert(false, "iOS does not have a language dialog.");
    return NULL;
}

MenuDialog* MenuBuilderIOS::buildDisplayDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    CCAssert(false, "iOS does not have a display dialog.");
    return NULL;
}

MenuDialog* MenuBuilderIOS::buildCreditsDialog(int category, IEventListener* listener, int evCode)
{
    MenuCustomDialog* dialog = MenuCustomDialog::create(category, 20, 0, CMInputs::HAS_FOCUS_MENU_DIALOG, listener, MenuCustomDialog::CREDITS);
    dialog->setCanGoBack(true);
    
    CCNode* bg = createDialogBg(640.0f, 420.0f);
    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    dialog->setScrollBounds(CCRectMake(
                                       bgBounds.origin.x,
                                       bgBounds.origin.y + 0.25f * bgBounds.size.height,
                                       bgBounds.size.width,
                                       kCreditsScrollHeightFactor * bgBounds.size.height));
    
	CCLabelBMFont* credits = TextUtils::create("Credits", 60, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	credits->setPosition(ccp(0, -credits->boundingBox().size.height / 2));
	credits->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuOrange));
	dialog->addScrollingItem(credits);
    
    CCSprite* wizardProp = CCSprite::createWithSpriteFrame(getScene()->spriteFrameByName("12.wizard"));
    wizardProp->setPosition(ccp(
                                bgBounds.size.width / 2 - (wizardProp->boundingBox().size.width / 2 + 30),
                                -wizardProp->boundingBox().size.height / 3));
	dialog->addScrollingItem(wizardProp);
    
    float roleSpacer = 1.25f * credits->boundingBox().size.height;
    
    // Game Design
    CCLabelBMFont* design = TextUtils::create("Game Design", MenuBuilder::kMenuFontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	design->setPosition(ccp(0, credits->getPositionY() - roleSpacer));
	design->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addScrollingItem(design);
    
    CCLabelBMFont* designerA = TextUtils::create("Adrian McPhee", 40, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	designerA->setPosition(ccp(0, design->getPositionY() - design->boundingBox().size.height));
	designerA->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(designerA);
    
    CCLabelBMFont* designerB = TextUtils::create("Paul McPhee", 40,
                                                     kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	designerB->setPosition(ccp(0, designerA->getPositionY() - designerA->boundingBox().size.height));
	designerB->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(designerB);
    
    // Code
    CCLabelBMFont* coding = TextUtils::create("Code", MenuBuilder::kMenuFontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	coding->setPosition(ccp(0, designerB->getPositionY() - roleSpacer));
	coding->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addScrollingItem(coding);
    
    CCLabelBMFont* coder = TextUtils::create("Paul McPhee", 40, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	coder->setPosition(ccp(0, coding->getPositionY() - coding->boundingBox().size.height));
	coder->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(coder);
    
    // Art
    CCLabelBMFont* art = TextUtils::create("Art", MenuBuilder::kMenuFontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	art->setPosition(ccp(0, coder->getPositionY() - roleSpacer));
	art->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addScrollingItem(art);
    
    CCLabelBMFont* artistA = TextUtils::create("Chen Shin", 40, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	artistA->setPosition(ccp(0, art->getPositionY() - art->boundingBox().size.height));
	artistA->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(artistA);
    
    CCLabelBMFont* artistB = TextUtils::create("Dean Spencer", 40, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	artistB->setPosition(ccp(0, artistA->getPositionY() - artistA->boundingBox().size.height));
	artistB->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(artistB);
    
    CCLabelBMFont* artistC = TextUtils::create("Talia Tsur", 40, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	artistC->setPosition(ccp(0, artistB->getPositionY() - artistB->boundingBox().size.height));
	artistC->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(artistC);
    
    // Sound Design
    CCLabelBMFont* soundDesign = TextUtils::create("Sound Design", MenuBuilder::kMenuFontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	soundDesign->setPosition(ccp(0, artistC->getPositionY() - roleSpacer));
	soundDesign->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addScrollingItem(soundDesign);
    
    CCLabelBMFont* soundDesigner = TextUtils::create("Daniel Beck", 40, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	soundDesigner->setPosition(ccp(0, soundDesign->getPositionY() - soundDesign->boundingBox().size.height));
	soundDesigner->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(soundDesigner);
    
    // UI Design
    CCLabelBMFont* uiDesign = TextUtils::create("UI Design", MenuBuilder::kMenuFontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	uiDesign->setPosition(ccp(0, soundDesigner->getPositionY() - roleSpacer));
	uiDesign->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addScrollingItem(uiDesign);
    
    CCLabelBMFont* uiDesigner = TextUtils::create("Sinclair C.", 40, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	uiDesigner->setPosition(ccp(0, uiDesign->getPositionY() - uiDesign->boundingBox().size.height));
	uiDesigner->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(uiDesigner);
    
    // QA Lead
    CCLabelBMFont* qaLead = TextUtils::create("QA Lead", MenuBuilder::kMenuFontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	qaLead->setPosition(ccp(0, uiDesigner->getPositionY() - roleSpacer));
	qaLead->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addScrollingItem(qaLead);
    
    CCLabelBMFont* qaLeader = TextUtils::create("Sinclair C.", 40, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	qaLeader->setPosition(ccp(0, qaLead->getPositionY() - qaLead->boundingBox().size.height));
	qaLeader->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(qaLeader);
    
    // Music
    CCLabelBMFont* music = TextUtils::create("Music", MenuBuilder::kMenuFontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	music->setPosition(ccp(0, qaLeader->getPositionY() - roleSpacer));
	music->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addScrollingItem(music);
    
    CCLabelBMFont* musician_0 = TextUtils::create("\"The tale of room 620\"\nEhren Starks & Kate Gurba", 36, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	musician_0->setPosition(ccp(0, music->getPositionY() - (music->boundingBox().size.height + musician_0->boundingBox().size.height / 3)));
	musician_0->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(musician_0);
    
    CCLabelBMFont* musician_1 = TextUtils::create("\"Sunset in Pensacola\"\nEhren Starks & Kate Gurba", 36, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	musician_1->setPosition(ccp(0, musician_0->getPositionY() - 1.5f * musician_0->boundingBox().size.height));
	musician_1->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(musician_1);
    
    CCLabelBMFont* musician_2 = TextUtils::create("\"Slippolska\"\nErik Ask Upmark", 36, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	musician_2->setPosition(ccp(0, musician_1->getPositionY() - 1.5f * musician_1->boundingBox().size.height));
	musician_2->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(musician_2);
    
    CCLabelBMFont* musician_3 = TextUtils::create("\"Florellen\"\nErik Ask Upmark", 36, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	musician_3->setPosition(ccp(0, musician_2->getPositionY() - 1.5f * musician_2->boundingBox().size.height));
	musician_3->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(musician_3);

    CCLabelBMFont* musician_4 = TextUtils::create("\"Blekingarna\"\nErik Ask Upmark", 36, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
    musician_4->setPosition(ccp(0, musician_3->getPositionY() - 1.5f * musician_3->boundingBox().size.height));
    musician_4->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
    dialog->addScrollingItem(musician_4);
    
//    CCLabelBMFont* musician_4 = TextUtils::create("\"Hidden Sky\"\nJami Sieber", 36, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
//	musician_4->setPosition(ccp(0, musician_3->getPositionY() - 1.5f * musician_3->boundingBox().size.height));
//	musician_4->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
//	dialog->addScrollingItem(musician_4);
    
    // A game by Cheeky Mammoth
    CCLabelBMFont* byLineA = TextUtils::create("a game by", MenuBuilder::kMenuFontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	byLineA->setPosition(ccp(0, musician_4->getPositionY() - 1.5f * roleSpacer));
	byLineA->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addScrollingItem(byLineA);
    
    CCLabelBMFont* byLineB = TextUtils::create("Cheeky Mammoth", 40, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	byLineB->setPosition(ccp(0, byLineA->getPositionY() - byLineA->boundingBox().size.height));
	byLineB->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuEnableColor));
	dialog->addScrollingItem(byLineB);
    
    CCSprite* logo = CCSprite::createWithSpriteFrame(getScene()->spriteFrameByName("mascot-happy"));
    logo->setScale(96.0f / logo->boundingBox().size.width);
    logo->setPosition(ccp(
                          byLineB->getPositionX() + byLineB->boundingBox().size.width / 2 + logo->boundingBox().size.width / 2,
                          byLineB->getPositionY() + (0.95f * logo->boundingBox().size.height - byLineB->boundingBox().size.height) / 2));
	dialog->addScrollingItem(logo);
    
    MenuItem* back = MenuBuilderIOS::createMenuItem("back", evCode);
    back->setPosition(ccp(0, 1.25f * back->boundingBox().size.height - bgBounds.size.height / 2));
	dialog->addMenuItem(back);
    
#if !CM_BETA
    dialog->enableCustomLayout(kUIDCredits, customLayoutFunctionIOS);
    dialog->addCustomLayoutItem(dialog);
    dialog->addCustomLayoutItem(bg);
    dialog->addCustomLayoutItem(wizardProp);
    dialog->layoutCustomItems();
#endif
    
    dialog->setShowScale(dialogScaleFactor());
    return dialog;
}

MenuDialog* MenuBuilderIOS::buildQueryDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
	// kEvYes, kEvNo
	CCAssert(evCodes.size() == 2, "MenuBuilder::buildQueryDialog - bad args.");
	MenuDialog* dialog = MenuDialog::create(category, 1, CMInputs::HAS_FOCUS_MENU_DIALOG, NAV_HORIZ, listener);
	dialog->setCanGoBack(true);
    
    CCNode* bg = createDialogBg(500.0f, 170.0f);
    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    
	CCLabelBMFont* caption = TextUtils::create("Are you sure?", MenuBuilder::kMenuFontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	caption->setPosition(ccp(0, bgBounds.size.height / 2 - 32));
	caption->setColor(CMUtils::uint2color3B(0xe08214));
	dialog->addContentItem(caption);
    
	MenuItem* yesItem = MenuBuilderIOS::createMenuItem("Yes", evCodes[0]);
	yesItem->setPosition(ccp(-0.175f * bgBounds.size.width, caption->getPositionY() - 70));
	dialog->addMenuItem(yesItem);
    
	MenuItem* noItem = MenuBuilderIOS::createMenuItem("No", evCodes[1]);
	noItem->setPosition(ccp(0.175f * bgBounds.size.width, yesItem->getPositionY()));
	dialog->addMenuItem(noItem);
    
    dialog->setShowScale(dialogScaleFactor());
	return dialog;
}

MenuDialog* MenuBuilderIOS::buildCloudQueryDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    // kEvYes, kEvNo
	CCAssert(evCodes.size() == 2, "MenuBuilder::buildCloudQueryDialog - bad args.");
	MenuDialog* dialog = MenuDialog::create(category, 1, CMInputs::HAS_FOCUS_MENU_DIALOG, NAV_HORIZ, listener);
	dialog->setCanGoBack(true);
    
    CCNode* bg = createDialogBg(600.0f, 220.0f);
    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    
	CCLabelBMFont* caption = TextUtils::create("Store game progress in iCloud?",
                                               36,
                                               kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	caption->setPosition(ccp(0, bgBounds.size.height / 2 - 56));
	caption->setColor(CMUtils::uint2color3B(0xe08214));
	dialog->addContentItem(caption);
    
	MenuItem* yesItem = MenuBuilderIOS::createMenuItem("Use iCloud", evCodes[0]);
	yesItem->setPosition(ccp(-0.2f * bgBounds.size.width, caption->getPositionY() - (caption->boundingBox().size.height + 32)));
	dialog->addMenuItem(yesItem);
    
	MenuItem* noItem = MenuBuilderIOS::createMenuItem("Local Only", evCodes[1]);
	noItem->setPosition(ccp(0.2f * bgBounds.size.width, yesItem->getPositionY()));
	dialog->addMenuItem(noItem);
    
    dialog->enableCustomLayout(kUIDCloudQuery, customLayoutFunctionIOS);
    dialog->addCustomLayoutItem(bg);
    dialog->addCustomLayoutItem(yesItem);
    dialog->addCustomLayoutItem(noItem);
    dialog->layoutCustomItems();
    
    dialog->setShowScale(dialogScaleFactor());
	return dialog;
}

MenuDialog* MenuBuilderIOS::buildLevelUnlockedDialog(int category, int levelIndex, IEventListener* listener, int evCode)
{
	// kEvLevelUnlocked
    MenuDialog* dialog = MenuCustomDialog::create(
                                                  category,
                                                  40,
                                                  levelIndex,
                                                  CMInputs::HAS_FOCUS_MENU_DIALOG,
                                                  listener,
                                                  levelIndex == LevelMenu::kNumLevels - 1 ? MenuCustomDialog::WIZARD_UNLOCKED : MenuCustomDialog::LEVEL_UNLOCKED);
    
    CCNode* bg = createDialogBg(800.0f, 600.0f);
    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    
    //	CCLabelBMFont* solvedLabel = CCLabelBMFont::create("3/6", getScene()->getFontName(52).c_str(), kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
    //	solvedLabel->setScale(60.0f / 52.0f);
    //	solvedLabel->setPosition(ccp(0, bgBounds.size.height / 2 - (20 + solvedLabel->boundingBox().size.height / 2)));
    //	solvedLabel->setColor(CMUtils::uint2color3B(0x00ff7a));
    //	dialog->addContentItem(solvedLabel);
    //
    //	CCSprite* solvedSprite = CCSprite::createWithSpriteFrame(getScene()->spriteFrameByName("menu-key"));
    //	solvedSprite->setPosition(ccp(
    //		solvedLabel->boundingBox().getMinX() + solvedLabel->boundingBox().size.width + solvedSprite->boundingBox().size.width / 2,
    //		solvedLabel->getPositionY()));
    //	dialog->addContentItem(solvedSprite);
    
	CCLabelBMFont* title = TextUtils::create("New level unlocked!", 52, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	//title->setPosition(ccp(0, solvedLabel->boundingBox().getMinY() - title->boundingBox().size.height / 2));
    title->setPosition(ccp(0, bgBounds.size.height / 2 - (36 + title->boundingBox().size.height / 2)));
	title->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addContentItem(title);
    
//	LevelIcon* levelIcon = new LevelIcon(category, levelIndex, -1);
//	levelIcon->init();
//	levelIcon->setPosition(ccp(0, -4));
//	//levelIcon->setLevelTextColor(MenuBuilder::kMenuBuilderSlateBlue);
//	levelIcon->setPuzzlesSolvedColor(MenuBuilder::kMenuBuilderSlateBlue);
//	levelIcon->setLocked(false);
//	dialog->addContentItem(levelIcon);
    
	MenuItem* okItem = MenuBuilderIOS::createMenuItem("OK", evCode);
    okItem->setScale(1.15f);
	okItem->setPosition(ccp(0, 1.0f * okItem->boundingBox().size.height - bgBounds.size.height / 2));
	dialog->addMenuItem(okItem);
    
	return dialog;
}

MenuDialog* MenuBuilderIOS::buildLevelCompletedDialog(int category, int levelIndex, IEventListener* listener, int evCode)
{
	// kEvLevelCompleted
	MenuCustomDialog* dialog = MenuCustomDialog::create(category, 40, levelIndex, CMInputs::HAS_FOCUS_MENU_DIALOG, listener, MenuCustomDialog::LEVEL_COMPLETE);
    
    CCNode* bg = createDialogBg(740.0f, 640.0f);
    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    
	CCLabelBMFont* titleLabel = TextUtils::create("Congratulations!", 64, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	titleLabel->setPosition(ccp(0, bgBounds.size.height / 2 - (24 + titleLabel->boundingBox().size.height / 2)));
	titleLabel->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addContentItem(titleLabel);
    
	if (levelIndex >= LevelMenu::kNumLevels)
		levelIndex = 0;
    
	CCSprite* levelSprite = CCSprite::createWithSpriteFrame(getScene()->spriteFrameByName(LevelMenu::kLevelTextureNames[levelIndex]));
	levelSprite->setScale(1.4f);
	levelSprite->setPosition(ccp(
                                 0,
                                 titleLabel->getPositionY() - 170));
	dialog->addContentItem(levelSprite);
    
	CCLabelBMFont* lowerLabel = TextUtils::create("Level", 46, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
    lowerLabel->setTag(MenuCustomDialog::kLevelCompleteLabelTagA);
	lowerLabel->setPosition(ccp(
                                0,
                                levelSprite->getPositionY() - 210));
	lowerLabel->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addContentItem(lowerLabel);
    
	CCLabelBMFont* lowestLabel = TextUtils::create("Completed", 46, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
    lowestLabel->setTag(MenuCustomDialog::kLevelCompleteLabelTagB);
	lowestLabel->setPosition(ccp(
                                 0,
                                 lowerLabel->getPositionY() - lowerLabel->boundingBox().size.height));
	lowestLabel->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addContentItem(lowestLabel);
    
    dialog->repositionLevelCompleteLabels();
    
	MenuItem* okItem = MenuBuilderIOS::createMenuItem("Yay", evCode);
    okItem->setScale(1.15f);
	okItem->setPosition(ccp(0, 1.1f * okItem->boundingBox().size.height - bgBounds.size.height / 2));
	dialog->addMenuItem(okItem);
    
	return dialog;
}

MenuDialog* MenuBuilderIOS::buildPuzzleWizardDialog(int category, IEventListener* listener, int evCode)
{
	// kEvPuzzleWizard
	MenuDialog* dialog = MenuCustomDialog::create(category, 40, 11, CMInputs::HAS_FOCUS_MENU_DIALOG, listener, MenuCustomDialog::WIZARD);
    
    CCNode* bg = createDialogBg(740.0f, 650.0f);
    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    
	CCLabelBMFont* titleLabel = TextUtils::create("Congratulations!", 64, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	titleLabel->setPosition(ccp(0, bgBounds.size.height / 2 - 3.25f * kDefaultVertPadding));
	titleLabel->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addContentItem(titleLabel);
    
	CCSprite* wizardSprite = CCSprite::createWithSpriteFrame(getScene()->spriteFrameByName("12.wizard"));
	wizardSprite->setScale(1.6f);
	wizardSprite->setPosition(ccp(
                                  0,
                                  titleLabel->getPositionY() - 0.7f * wizardSprite->boundingBox().size.height));
	dialog->addContentItem(wizardSprite);
    
	CCLabelBMFont* midLabel = TextUtils::create("Puzzle Wizard", 60, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	midLabel->setPosition(ccp(
                              0,
                              wizardSprite->getPositionY() - 0.6f * wizardSprite->boundingBox().size.height));
	midLabel->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addContentItem(midLabel);
    
	CCLabelBMFont* lowerLabel = TextUtils::create("Master of Color", 42, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	lowerLabel->setPosition(ccp(
                                0,
                                midLabel->getPositionY() - 0.9f * midLabel->boundingBox().size.height));
	lowerLabel->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addContentItem(lowerLabel);
    
	MenuItem* okItem = MenuBuilderIOS::createMenuItem("Yay", evCode);
    okItem->setScale(1.15f);
	okItem->setPosition(ccp(0, 1.35f * okItem->boundingBox().size.height - bgBounds.size.height / 2));
	dialog->addMenuItem(okItem);
    
	return dialog;
}

MenuDialog* MenuBuilderIOS::buildRateTheGameDialog(int category, IEventListener* listener, const std::vector<int>& evCodes)
{
    // kEvRateYes, kEvRateNo
    CCAssert(evCodes.size() == 2, "MenuBuilder::buildRateTheGameDialog - bad args.");
	MenuDialog* dialog = MenuDialog::create(category, 1, CMInputs::HAS_FOCUS_MENU_DIALOG, NAV_HORIZ, listener);
    
    CCNode* bg = createDialogBg(760.0f, 360.0f);
    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    
	CCLabelBMFont* caption = TextUtils::create("Please rate our game if you enjoy it.",
                                               36,
                                               kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	caption->setPosition(ccp(0, bgBounds.size.height / 2 - 56));
	caption->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addContentItem(caption);
    
	MenuItem* yesItem = MenuBuilderIOS::createMenuItem("Yes rate it!", evCodes[0]);
	yesItem->setPosition(ccp(-0.2f * bgBounds.size.width, caption->getPositionY() - 220));
	dialog->addMenuItem(yesItem);
    
    CCSprite* yesIcon = CCSprite::createWithSpriteFrame(getScene()->spriteFrameByName("mascot-happy"));
    yesIcon->setScaleX(-1);
    yesIcon->setPosition(ccp(
                             yesItem->getPositionX(),
                             yesItem->getPositionY() + (0.9f * yesItem->boundingBox().size.height + yesIcon->boundingBox().size.height) / 2));
    dialog->addContentItem(yesIcon);
    
	MenuItem* noItem = MenuBuilderIOS::createMenuItem("Not now...", evCodes[1]);
	noItem->setPosition(ccp(0.2f * bgBounds.size.width, yesItem->getPositionY()));
	dialog->addMenuItem(noItem);
    
    CCSprite* noIcon = CCSprite::createWithSpriteFrame(getScene()->spriteFrameByName("mascot-sad"));
    noIcon->setPosition(ccp(
                            noItem->getPositionX() + 0.2f * noIcon->boundingBox().size.width,
                            noItem->getPositionY() + (0.9f * noItem->boundingBox().size.height + noIcon->boundingBox().size.height) / 2));
    dialog->addContentItem(noIcon);
    
    dialog->setShowScale(dialogScaleFactor());
	return dialog;
}

MenuDialog* MenuBuilderIOS::buildBetaReportBugThanksDialog(int category, IEventListener* listener, int evCode)
{
    MenuDialog* dialog = MenuDialog::create(category, 1, CMInputs::HAS_FOCUS_MENU_DIALOG, NAV_HORIZ, listener);
	dialog->setCanGoBack(true);
    
    CCNode* bg = createDialogBg(480.0f, 260.0f);
    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    
	CCLabelBMFont* caption = TextUtils::create("Report submitted.\nThank you!", MenuBuilder::kMenuFontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
	caption->setPosition(ccp(0, bgBounds.size.height / 2 - 80));
	caption->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addContentItem(caption);
    
	MenuItem* okItem = MenuBuilderIOS::createMenuItem("OK", evCode);
    okItem->setScale(1.15f);
	okItem->setPosition(ccp(0, 1.25f * okItem->boundingBox().size.height - bgBounds.size.height / 2));
	dialog->addMenuItem(okItem);
    
    dialog->setShowScale(dialogScaleFactor());
	return dialog;
}

MenuDialog* MenuBuilderIOS::buildBetaIQRatingDialog(int category, IEventListener* listener, int evCode)
{
    MenuDialog* dialog = MenuDialog::create(category, 1, CMInputs::HAS_FOCUS_MENU_DIALOG, NAV_HORIZ, listener);
	dialog->setCanGoBack(true);
    
    CCNode* bg = createDialogBg(720.0f, 460.0f);
    dialog->addBgItem(bg);
    
	CCRect bgBounds = bg->boundingBox();
    
	CCLabelBMFont* caption = TextUtils::create(
                                               "Please adjust the IQ rating for\nthis puzzle if you disagree with\nits present value of 000.",
                                               MenuBuilder::kMenuFontSize,
                                               kCCLabelAutomaticWidth,
                                               kCCTextAlignmentCenter,
                                               CCPointZero);
    caption->setTag(evCode);
	caption->setPosition(ccp(0, bgBounds.size.height / 2 - 100));
	caption->setColor(CMUtils::uint2color3B(MenuBuilder::kMenuDarkYellow));
	dialog->addContentItem(caption);
    
    IQSlider* slider = IQSlider::create(50, 160, listener);
    slider->setTag(IQSlider::EV_TYPE_IQ_CHANGED());
    slider->setPosition(ccp(0, -20));
    dialog->addContentItem(slider);
    dialog->addTouchableItem(slider);
    
	MenuItem* submitItem = MenuBuilderIOS::createMenuItem("Submit", evCode);
	submitItem->setPosition(ccp(0, 1.4f * submitItem->boundingBox().size.height - bgBounds.size.height / 2));
	dialog->addMenuItem(submitItem);
    
    //dialog->setShowScale(dialogScaleFactor());
	return dialog;
}
