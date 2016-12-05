#ifndef __LANGFX_H__
#define __LANGFX_H__

#include "cocos2d.h"
#include "Localizer.h"
USING_NS_CC;

namespace LangFX
{
    // Font notes:
        // EU-52.fnt : lineHeight=144 -> 106 == 0.7361
        // IQ-48.fnt : lineHeight=133 -> 112 == 0.8421
    
    typedef struct {
        float val_0;
        float val_1;
    } Offset_2;
    
    typedef struct {
        float val_0;
        float val_1;
        float val_2;
    } Offset_3;
    
    typedef struct
    {
        float puzzleHorizSeparation;
        float puzzleVertSeparation;
        float puzzleBoardWidth;
        float puzzleBoardHeight;
        float puzzleEntryYOffset;
        float headerImageYOffsetFactor;
        float defaultOffsetY;
        
    } PuzzlePageSettings;
    
    // -------------- Splash Offsets/Utils --------------
    CCPoint getSplashLogoOffset(Localizer::LocaleType locale);
    CCPoint getSplashTextPuzzleOffset(Localizer::LocaleType locale);
    CCPoint getSplashTextWizardOffset(Localizer::LocaleType locale);
    CCPoint getSplashTextIqOffset(Localizer::LocaleType locale);
    const char* locale2IQString(Localizer::LocaleType locale, bool uppercase);
    const char* locale2PuzzleTexSuffix(Localizer::LocaleType locale);
    const char* locale2WizardTexSuffix(Localizer::LocaleType locale);
    
    // -------------- General Offsets --------------
    float getActiveIconYOffset(char firstChar = ' ');
    float getGaugeStrokeYOffset(void);
    CCPoint getSolvedPromptLabelOffset(void);
    Offset_2 getPlayerHUDSettings(void);
    
    // -------------- LevelMenu Offsets --------------
    CCSprite* getLevelMenuHeader(void);
    CCSize getLevelMenuHeaderSizeOSX(void);
    Offset_2 getLevelIconYOffsets(void);
    Offset_3 getLevelIconSolvedOffsets(void);
    float getLevelMenuHeaderLabelYOffset(void);
    float getLevelMenuOverlayYOffset(void);
    Offset_2 getLevelMenuHelpUnlockYOffsets(void);
    Offset_2 getLevelMenuTotalProgressOffsets(void);
    const char* getLevelMenuHelpUnlockPaddingString(void);
    PuzzlePageSettings getPuzzlePageSettings(void);
    CCPoint getMaxPuzzlePageBoardDimensions(void);
    Offset_2 getPuzzleMenuBackButtonIconOffset(void);
    
    // -------------- PuzzleMenu Offsets --------------
    float getPuzzleLabelYOffset(void);
    
    
    // -------------- MenuCustomDialog Offsets --------------
    float getCustomDialogStarYOffset(void);
    float getCustomDialogHeaderYOffset(void);
    float getCustomDialogKeyYOffset(void);
    Offset_2 getLevelCompleteLabelOffsets(Localizer::LocaleType locale);
    
    // -------------- Misc Offsets --------------
    CCPoint getTitleTextOffset(void);
}

#endif // __LANGFX_H__
