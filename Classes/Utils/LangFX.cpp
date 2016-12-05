
#include "LangFX.h"
#include "support/ccUtils.h"
#include <Utils/DeviceDetails.h>
USING_NS_CC;

namespace LangFX
{
    // -------------- Splash Offsets --------------
    static const CCPoint kSplashLogoOffsets[] = {
        ccp(0, 48),     // EN
        ccp(0, 40),    // CN
        ccp(0, 40),    // DE
        ccp(0, 40),    // ES
        ccp(0, 40),     // FR
        ccp(0, 40),    // IT
        ccp(0, 40),    // JP
        ccp(0, 40)     // KR
    };
    CCPoint getSplashLogoOffset(Localizer::LocaleType locale)
    {
        return kSplashLogoOffsets[locale];
    }
    
    static const CCPoint kSplashTextPuzzleOffsets[] = {
        ccp(3, -118),     // EN
        ccp(0, -165),    // CN
        ccp(0, -110),    // DE
        ccp(0, -110),    // ES
        ccp(0, -92),     // FR
        ccp(0, -110),    // IT
        ccp(0, -105),    // JP
        ccp(0, -165)     // KR
    };
    CCPoint getSplashTextPuzzleOffset(Localizer::LocaleType locale)
    {
        return kSplashTextPuzzleOffsets[locale];
    }
    
    static const CCPoint kSplashTextWizardOffsets[] = {
        ccp(-2, -180),   // EN
        ccp(0, 0),       // CN
        ccp(0, -176),    // DE
        ccp(0, -176),    // ES
        ccp(0, -170),    // FR
        ccp(0, -176),    // IT
        ccp(0, -176),    // JP
        ccp(0, 0)        // KR
    };
    CCPoint getSplashTextWizardOffset(Localizer::LocaleType locale)
    {
        return kSplashTextWizardOffsets[locale];
    }
    
    static const CCPoint kSplashTextIqOffsets[] = {
        ccp(0, -262),    // EN
        ccp(0, -252),    // CN
        ccp(0, -252),    // DE
        ccp(0, -252),    // ES
        ccp(0, -252),    // FR
        ccp(0, -252),    // IT
        ccp(0, -252),    // JP
        ccp(0, -252)     // KR
    };
    CCPoint getSplashTextIqOffset(Localizer::LocaleType locale)
    {
        return kSplashTextIqOffsets[locale];
    }
    
    const char* locale2IQString(Localizer::LocaleType locale, bool uppercase)
    {
        const char* iqStr = 0;
        
        switch (locale) {
            case Localizer::FR:
            case Localizer::IT:
                iqStr = "qi";
                break;
            case Localizer::ES:
                iqStr = "ci";
                break;
            default:
                iqStr = "iq";
                break;
        }
        
        return iqStr;
    }
    
    const char* locale2PuzzleTexSuffix(Localizer::LocaleType locale)
    {
        const char* texStr = 0;
        
        switch (locale) {
            case Localizer::IT:
            case Localizer::JP:
            case Localizer::ES:
            case Localizer::CN:
            case Localizer::KR:
                texStr = Localizer::locale2String(locale);
                break;
            default:
                texStr = "EN";
                break;
        }
        
        return texStr;
    }
    
    const char* locale2WizardTexSuffix(Localizer::LocaleType locale)
    {
        const char* texStr = 0;
        
        switch (locale) {
            case Localizer::ES:
            case Localizer::IT:
            case Localizer::DE:
            case Localizer::FR:
            case Localizer::JP:
                texStr = Localizer::locale2String(locale);
                break;
            case Localizer::CN:
            case Localizer::KR:
                texStr = 0;
                break;
            default:
                texStr = "EN";
                break;
        }
        
        return texStr;
    }
    
    // -------------- General Offsets --------------
    static const Offset_2 kActiveIconYOffsets[] = {
        // { alpha, numeric }
        { -2 , 0 },      // EN
        { 0, 0 },      // CN
        { -2, 0 },      // DE
        { -2, 0 },      // ES
        { -2, 0 },      // FR
        { -2, 0 },      // IT
        { 2, 0 },      // JP
        { 3, 3 }       // KR
    };
    float getActiveIconYOffset(char firstChar)
    {
        float offsetY = kActiveIconYOffsets[Localizer::getLocale()].val_0;
        if (firstChar >= '0' && firstChar <= '9')
            offsetY += kActiveIconYOffsets[Localizer::getLocale()].val_1;
        return offsetY;
    }
    
    static const float kGaugeStrokeYOffsets[] = {
        1,      // EN
        2,      // CN
        1,      // DE
        1,      // ES
        1,      // FR
        1,      // IT
        4,      // JP
        4       // KR
    };
    float getGaugeStrokeYOffset(void)
    {
        return kGaugeStrokeYOffsets[Localizer::getLocale()];
    }
    
    static const CCPoint kSolvedPromptLabelOffsets[] = {
        ccp(-6, -16),  // EN
        ccp(-6, 0),  // CN
        ccp(-6, -16),  // DE
        ccp(-6, -16),  // ES
        ccp(-6, -16),  // FR
        ccp(-6, -16),  // IT
        ccp(-6, -5),  // JP
        ccp(-6, 4)   // KR
    };
    CCPoint getSolvedPromptLabelOffset(void)
    {
        return kSolvedPromptLabelOffsets[Localizer::getLocale()];
    }
    
    static const Offset_2 kPlayerHUDSettings[] = {
        // { LabelYOffset, BgQuadWidth }
        { 1, 500 },      // EN
        { -1, 330 },      // CN
        { 1, 510 },      // DE
        { 1, 580 },      // ES
        { 1, 630 },      // FR
        { 1, 460 },      // IT
        { -2, 430 },      // JP
        { -4, 380 }       // KR
    };
    Offset_2 getPlayerHUDSettings(void)
    {
        return kPlayerHUDSettings[Localizer::getLocale()];
    }
    
    // -------------- LevelMenu Offsets --------------
    CCSprite* getLevelMenuHeader(void)
    {
        std::string headerTexKey = std::string("lang/title-").append(Localizer::locale2String(Localizer::getLocale())).append(".png");
        CCTexture2D* headerTex = CCTextureCache::sharedTextureCache()->textureForKey(headerTexKey.c_str());
        if (headerTex == NULL)
        {
            headerTex = CCTextureCache::sharedTextureCache()->addImage(headerTexKey.c_str());
#ifdef CHEEKY_DESKTOP
            CCSize texSize = headerTex->getContentSizeInPixels();
            
            if ((int)texSize.width == ccNextPOT((int)texSize.width) && (int)texSize.height == ccNextPOT((int)texSize.height))
            {
                ccTexParams params = {GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE};
                headerTex->setTexParameters(&params);
                headerTex->generateMipmap();
            }
#endif
        }
        
        CCSprite* headerSprite = CCSprite::createWithTexture(headerTex);
//#ifdef CHEEKY_DESKTOP
//        headerSprite->setAnchorPoint(ccp(0.5f, MAX(0.0f, 0.5f - 0.5f * (getLevelMenuHeaderSizeOSX().height / headerSprite->boundingBox().size.height))));
//#else
//        headerSprite->setAnchorPoint(ccp(0.5f, 0.0f));
//#endif
        return headerSprite;
    }
    
    static const CCSize kLevelMenuHeaderSizesOSX[] = {
        CCSizeMake(512, 256),      // EN
        CCSizeMake(512, 132),      // CN
        CCSizeMake(512, 200),      // DE
        CCSizeMake(512, 256),      // ES
        CCSizeMake(512, 256),      // FR
        CCSizeMake(512, 256),      // IT
        CCSizeMake(512, 238),      // JP
        CCSizeMake(512, 110)       // KR
    };
    CCSize getLevelMenuHeaderSizeOSX(void)
    {
        return kLevelMenuHeaderSizesOSX[Localizer::getLocale()];
    }
    
    static const Offset_2 kLevelMenuIconYOffsets[] = {
        // { Pinstripe, Label }
        { 84, 76 },      // EN
        { 84, 74 },      // CN
        { 84, 76 },      // DE
        { 84, 76 },      // ES
        { 84, 76 },      // FR
        { 84, 76 },      // IT
        { 84, 74 },      // JP
        { 84, 72 }       // KR
    };
    Offset_2 getLevelIconYOffsets(void)
    {
        return kLevelMenuIconYOffsets[Localizer::getLocale()];
    }
    
    static const Offset_3 kLevelMenuIconSolvedOffsets[] = {
        // { Text X, Text Y, Key Y }
        { -24, -92, -1 },      // EN
        { -24, -94, 2 },      // CN
        { -24, -92, -1 },      // DE
        { -24, -92, -1 },      // ES
        { -24, -92, -1 },      // FR
        { -24, -92, -1 },      // IT
        { -23, -95, 3 },      // JP
        { -24, -96, 3 }       // KR
    };
    Offset_3 getLevelIconSolvedOffsets(void)
    {
        return kLevelMenuIconSolvedOffsets[Localizer::getLocale()];
    }
    
    static const float kLevelMenuHeaderLabelYOffsets[] = {
        14,      // EN
        16,      // CN
        14,      // DE
        14,      // ES
        14,      // FR
        14,      // IT
        8,      // JP
        2       // KR
    };
    float getLevelMenuHeaderLabelYOffset(void)
    {
        return kLevelMenuHeaderLabelYOffsets[Localizer::getLocale()];
    }
    
    static const float kLevelMenuOverlayYOffsets[] = {
        -12,      // EN
        -30,      // CN
        -12,      // DE
        -12,      // ES
        -12,      // FR
        -12,      // IT
        -22,      // JP
        -24       // KR
    };
    float getLevelMenuOverlayYOffset(void)
    {
        return kLevelMenuOverlayYOffsets[Localizer::getLocale()];
    }
    
    static const Offset_2 kLevelMenuHelpUnlockYOffsets[] = {
        // { Container (incl text), Image }
        { -4, -4 },      // EN
        { -40, -2 },      // CN
        { -4, -4 },      // DE
        { -4, -4 },      // ES
        { -4, -4 },      // FR
        { -4, -4 },      // IT
        { -28, 0 },      // JP
        { -36, 2 }       // KR
    };
    Offset_2 getLevelMenuHelpUnlockYOffsets(void)
    {
        return kLevelMenuHelpUnlockYOffsets[Localizer::getLocale()];
    }
    
    static const Offset_2 kLevelMenuTotalProgressOffsets[] = {
        // { X, Y }
        { 2, 0 },      // EN
        { 16, 1 },      // CN
        { -2, 0 },      // DE
        { 2, 0 },      // ES
        { 3, 0 },      // FR
        { 2, 0 },      // IT
        { 1, 2 },      // JP
        { 9, 5 }       // KR
    };
    Offset_2 getLevelMenuTotalProgressOffsets(void)
    {
        return kLevelMenuTotalProgressOffsets[Localizer::getLocale()];
    }
    
    static const char* kLevelMenuHelpUnlockPaddingStrings[] = {
        "           ",    // EN
        "  ",             // CN
        "           ",    // DE
        "           ",    // ES
        "           ",    // FR
        "           ",    // IT
        "    ",           // JP
        "      "          // KR
    };
    const char* getLevelMenuHelpUnlockPaddingString(void)
    {
        return kLevelMenuHelpUnlockPaddingStrings[Localizer::getLocale()];
    }
    
    PuzzlePageSettings getPuzzlePageSettings(void)
    {
        PuzzlePageSettings settings;
//        Localizer::LocaleType locale = Localizer::getLocale();

//        if (locale == Localizer::EN || locale == Localizer::CN || locale == Localizer::JP || locale == Localizer::KR)
//        {
//            settings.puzzleHorizSeparation = 330; // 372;
//            settings.puzzleVertSeparation = 254;
//            settings.puzzleBoardWidth = getMaxPuzzlePageBoardDimensions().x;
//            settings.puzzleBoardHeight = getMaxPuzzlePageBoardDimensions().y;
//            settings.puzzleEntryYOffset = 320;
//            settings.headerImageYOffsetFactor = 1.15f;
//            settings.backButtonYOffsetFactor = 0.875f;
//        }
//        else
//        {
//            settings.puzzleHorizSeparation = 330; //372;
//            settings.puzzleVertSeparation = 254;
//            settings.puzzleBoardWidth = getMaxPuzzlePageBoardDimensions().x; //248;
//            settings.puzzleBoardHeight = getMaxPuzzlePageBoardDimensions().y; //162;
//            settings.puzzleEntryYOffset = 320;
//            settings.headerImageYOffsetFactor = 1.15f;
//            settings.backButtonYOffsetFactor = 0.875f;
//        }
        
        settings.puzzleHorizSeparation = 360; // 330;
        settings.puzzleVertSeparation = 254;
        settings.puzzleBoardWidth = getMaxPuzzlePageBoardDimensions().x;
        settings.puzzleBoardHeight = getMaxPuzzlePageBoardDimensions().y;
        settings.puzzleEntryYOffset = 320;
        settings.headerImageYOffsetFactor = 1.15f;
        settings.defaultOffsetY = 0;
        
        return settings;
    }
    
    CCPoint getMaxPuzzlePageBoardDimensions(void)
    {
        //return ccp(274, 180);
        //return ccp(260, 170);
        return MODE_8x6 ? ccp(260, 160) : ccp(250, 160);
    }
    
    static const Offset_2 kPuzzleMenuBackButtonIconOffsets[] = {
        // { X, Y }
        { 0, 0 },      // EN
        { -16, 2 },      // CN
        { 0, 0 },      // DE
        { 0, 0 },      // ES
        { 0, 0 },      // FR
        { 0, 0 },      // IT
        { -16, 2 },      // JP
        { -22, 4 }       // KR
    };
    Offset_2 getPuzzleMenuBackButtonIconOffset(void)
    {
        return kPuzzleMenuBackButtonIconOffsets[Localizer::getLocale()];
    }
    
    // -------------- PuzzleMenu Offsets --------------
    static const float kPuzzleMenuPuzzleLabelYOffsets[] = {
        0,      // EN
        0,      // CN
        0,      // DE
        0,      // ES
        0,      // FR
        0,      // IT
        0,      // JP
        -6       // KR
    };
    float getPuzzleLabelYOffset(void)
    {
        return kPuzzleMenuPuzzleLabelYOffsets[Localizer::getLocale()];
    }
    
    // -------------- MenuCustomDialog Offsets --------------
    static const float kCustomDialogStarYOffsets[] = {
        10,      // EN
        4,      // CN
        10,      // DE
        10,      // ES
        10,      // FR
        10,      // IT
        8,      // JP
        8       // KR
    };
    float getCustomDialogStarYOffset(void)
    {
        return kCustomDialogStarYOffsets[Localizer::getLocale()];
    }
    
    static const float kCustomDialogHeaderYOffsets[] = {
        -30,    // EN
        0,      // CN
        -30,    // DE
        -30,    // ES
        -30,    // FR
        -30,    // IT
        0,      // JP
        0       // KR
    };
    float getCustomDialogHeaderYOffset(void)
    {
        return kCustomDialogHeaderYOffsets[Localizer::getLocale()];
    }
    
    static const float kCustomDialogKeyYOffsets[] = {
        -2,    // EN
        0,    // CN
        -2,    // DE
        -2,    // ES
        -2,    // FR
        -2,    // IT
        2,    // JP
        2     // KR
    };
    float getCustomDialogKeyYOffset(void)
    {
        return kCustomDialogKeyYOffsets[Localizer::getLocale()];
    }
    
    static const Offset_2 kLevelCompleteLabelOffsets[] = {
        // { Upper Y, Lower Y }
        { -10, -10 },      // EN
        { -13, -14 },    // CN
        { -10, -10 },      // DE
        { -10, -10 },      // ES
        { -10, -10 },      // FR
        { -10, -10 },      // IT
        { -4, -5 },       // JP
        { -14, -15 }     // KR
    };
    Offset_2 getLevelCompleteLabelOffsets(Localizer::LocaleType locale)
    {
        return kLevelCompleteLabelOffsets[locale];
    }
    
    // -------------- Misc Offsets --------------
    static const CCPoint kTitleTextOffsets[] = {
        ccp(40, 1),     // EN
        ccp(48, 65),    // CN
        ccp(25, 20),    // DE
        ccp(125, 2),    // ES
        ccp(22, 1),     // FR
        ccp(125, 2),    // IT
        ccp(58, 10),    // JP
        ccp(26, 71)     // KR
    };
    CCPoint getTitleTextOffset(void)
    {
        return kTitleTextOffsets[Localizer::getLocale()];
    }
}

