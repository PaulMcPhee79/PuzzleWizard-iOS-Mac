
#include "LevelMenu.h"
#include <Prop/UIControls/IconButton.h>
#include <Prop/CroppedProp.h>
#include <Puzzle/View/Menu/LevelIcon.h>
#include <Puzzle/View/Menu/PuzzlePage.h>
#include <Puzzle/View/Menu/PuzzlePageEntry.h>
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Puzzle/View/Playfield/TileDecoration.h>
#include <Puzzle/View/Menu/LevelOverlay.h>
#include <Puzzle/Data/Level.h>
#include <Puzzle/Data/Puzzle.h>
#include <Utils/FloatTweener.h>
#include <Utils/ByteTweener.h>
#include <Prop/UIControls/RateIcon.h>
#include <Prop/ParticleProp.h>
#include <Puzzle/Inputs/TouchPad.h>
#include <Managers/InputManager.h>
#include <Managers/ControlsManager.h>
#include <Puzzle/Controllers/GameProgressController.h>
#include <SceneControllers/GameController.h>
#include <Utils/BridgingUtility.h>
#include <Utils/Utils.h>
#include <Utils/LangFX.h>
#include <cmath>
USING_NS_CC;

const char* LevelMenu::kLevelNames[kNumLevels] =
{
	"First Steps", "Color Swap", "Color Shield", "Conveyor Belt", "Rotator", "Color Flood",
	"White Tile", "Mirror Image", "Color Swirl", "Tile Swap", "Color Magic", "Wizard"
};

const char* LevelMenu::kLevelTextureNames[kNumLevels] =
{
	"1.first-steps", "2.color-swap", "3.color-shield", "4.conveyor-belt", "5.rotator", "6.color-flood",
	"7.white-tile", "8.mirror-image", "9.color-swirl", "10.tile-swap", "11.color-magic", "12.wizard"
};

const uint LevelMenu::kLevelColors[kNumLevels] =
{
	0x7dbbef, 0x44a4f6, 0x007ce5,
    0x77d5b9, 0x49d47d, 0x31c84e,
    0xfffd5a, 0xfffd33, 0xffff00,
    0xea8585, 0xf64444, 0xe50000
};


#if CM_BETA
static const char* kVersionStr = "v1.0.1 (2173)";
#endif

//static const uint kColorSlateBlue = 0x6a5acd;
#ifdef CHEEKY_MOBILE
//static const uint kColorCharcoal = 0x3a3a3a; //0x222222; //0x1a1a1a;
#else
//static const uint kColorCharcoal = 0x222222;
#endif
//static const uint kColorWhite = 0xffffff;

const float LevelMenu::kLevel2PuzzleTransitionDuration = 0.3f;
const float LevelMenu::kPuzzle2LevelTransitionDuration = 0.3f;
const float LevelMenu::kPuzzle2IdleTransitionDuration = 1.0f;

static const float kLevelIconWidth = 248.0f;
static const float kLevelPadding = 32.0f;
static const float kLevelWidth = 3 * kLevelIconWidth + 4 * kLevelPadding;
static const float kLevelSeparationX = kLevelIconWidth + kLevelPadding;
static const float kContentYScale = 2.15f;
static const float kScrollDist = kLevelWidth - kLevelPadding;

static const float kLevelScrollDuration = 0.7f;

static const int kNumLevelsPerPage = 3;
static const int kMaxLevels = 12;
static const int kNumPages = kMaxLevels / kNumLevelsPerPage;

#ifdef CHEEKY_MOBILE
static const float kMovedTouchThreshold = 2 * 16.0f;
static const int kScrollTouchThreshold = 2 * 15;
#endif

static const int kButtonTagPrev = 1;
static const int kButtonTagNext = 2;
static const int kButtonTagBack = 3;

static const int kTweenerTagScroll = 1;
static const int kTweenerTagContentOpacity = 2;
static const int kTweenerTagDecorationsOpacity = 3;
static const int kTweenerTagPuzzlePageOpacity = 4;
static const int kTweenerTagBgOpacity = 5;
static const int kTweenerTagHelpOpacity = 6;

//static const int kLeftCurtainIndex = 0;
//static const int kRightCurtainIndex = 1;
//static const int kNumCurtains = 2;

static const float kHelpFadeDelay = 4.0f;
static const float kHelpFadeDuration = 0.5f;


//#pragma warning( disable : 4351 ) // new behavior: elements of array 'xyz' will be default initialized (changed in Visual C++ 2005)
LevelMenu::LevelMenu(int category, CCArray* levels)
	:
	Prop(category),
	mState(LevelMenu::LEVELS),
	mQueuedState(LevelMenu::IDLE),
	mIsTouchEnabled(true),
	mIsTouched(false),
	mIsArrowTouched(false),
	mDidTouchSelect(false),
	mDidTouchMove(false),
    mDidTouchCancel(false),
    mIsScrolling(false),
	mUnlockedAll(false),
    mRepeatVec(CMCoordZero),
    mRepeatCounter(0),
    mRepeatDelay(0.25f),
	mLevelIndex(0),
	mPuzzleIndex(0),
	mPageIndex(0),
	mScrollDir(0),
	mScrollDelta(0),
	mScrollDeltaAbs(0),
	mHelpSprite(NULL),
	mHelpLabel(NULL),
	mHelpContainer(NULL),
    mHeaderQuad(NULL),
    mHeaderSprite(NULL),
    mFooterKey(NULL),
    mFooterLabel(NULL),
    mPrevPageArrow(NULL),
    mNextPageArrow(NULL),
    mDecorations(NULL),
    mLevelOverlay(NULL),
    mOverlayContainer(NULL),
    mRateIcon(NULL),
    mBackButton(NULL),
    mContent(NULL),
    mContentShadows(NULL),
    mCanvas(NULL),
    mBgCurtain(NULL),
    mPuzzlePage(NULL),
	mHelpOpacityTweener(NULL),
	mScrollTweener(NULL),
	mContentOpacityTweener(NULL),
	mDecorationsOpacityTweener(NULL),
	mPuzzlePageOpacityTweener(NULL),
	mBgOpacityTweener(NULL),
    mBgSelectButton(NULL)
{
	CCAssert(levels && levels->count() > 0, "LevelMenu requires non-NULL levels with count > 0.");
	levels->retain();
	mLevels = levels;
}

LevelMenu::~LevelMenu(void)
{
	mScene->deregisterForTouches(CMTouches::TC_MENU, this);
    mScene->deregisterResDependent(this);
    mScene->deregisterLocalizable(this);
    GameController::GC()->deregisterForEvent(GameController::EV_TYPE_USER_DID_RATE_THE_GAME(), this);
    CCDirector::sharedDirector()->getKeypadDispatcher()->removeDelegate(this);

	for (size_t i = 0; i < mLevelIcons.size(); ++i)
		CC_SAFE_RELEASE_NULL(mLevelIcons[i]);
	mLevelIcons.clear();
    
    for (size_t i = 0; i < mLevelIconShadows.size(); ++i)
		CC_SAFE_RELEASE_NULL(mLevelIconShadows[i]);
	mLevelIconShadows.clear();

	for (size_t i = 0; i < mLevelHeaders.size(); ++i)
		CC_SAFE_RELEASE_NULL(mLevelHeaders[i]);
	mLevelHeaders.clear();

	if (mScrollTweener)
		mScrollTweener->setListener(NULL);
	CC_SAFE_RELEASE_NULL(mScrollTweener);

	if (mContentOpacityTweener)
		mContentOpacityTweener->setListener(NULL);
	CC_SAFE_RELEASE_NULL(mContentOpacityTweener);

	if (mDecorationsOpacityTweener)
		mDecorationsOpacityTweener->setListener(NULL);
	CC_SAFE_RELEASE_NULL(mDecorationsOpacityTweener);

	if (mPuzzlePageOpacityTweener)
		mPuzzlePageOpacityTweener->setListener(NULL);
	CC_SAFE_RELEASE_NULL(mPuzzlePageOpacityTweener);

	if (mBgOpacityTweener)
		mBgOpacityTweener->setListener(NULL);
	CC_SAFE_RELEASE_NULL(mBgOpacityTweener);

	if (mHelpOpacityTweener)
		mHelpOpacityTweener->setListener(NULL);
	CC_SAFE_RELEASE_NULL(mHelpOpacityTweener);

	if (mBackButton)
		mBackButton->setListener(NULL);
	CC_SAFE_RELEASE_NULL(mBackButton);
    
    if (mRateIcon)
        mRateIcon->removeEventListener(RateIcon::EV_TYPE_RATE_ICON_PRESSED(), this);
    CC_SAFE_RELEASE_NULL(mRateIcon);
    
    CC_SAFE_RELEASE_NULL(mHeaderQuad);
	CC_SAFE_RELEASE_NULL(mHelpContainer);
	CC_SAFE_RELEASE_NULL(mDecorations);
    CC_SAFE_RELEASE_NULL(mLevelOverlay);
    CC_SAFE_RELEASE_NULL(mOverlayContainer);
	CC_SAFE_RELEASE_NULL(mContent);
    CC_SAFE_RELEASE_NULL(mContentShadows);
	CC_SAFE_RELEASE_NULL(mCanvas);
	CC_SAFE_RELEASE_NULL(mPuzzlePage);
	CC_SAFE_RELEASE_NULL(mLevels);
    CC_SAFE_RELEASE_NULL(mFooterLabel);
    CC_SAFE_RELEASE_NULL(mFooterKey);
    CC_SAFE_RELEASE_NULL(mBgSelectButton);
}

bool LevelMenu::init(void)
{
    const int kLabelReserveLen = 32;
	float contentX = mScene->getViewWidth() / 2, contentY = mScene->getViewHeight() / kContentYScale;
    
#if 0
	mBgCurtain = CMUtils::createColoredQuad(CCSizeMake(mScene->getFullscreenWidth(), mScene->getFullscreenHeight()));
	mBgCurtain->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
	//mBgCurtain->setColor(ccc3(0,0,0));
    mBgCurtain->setColor(CMUtils::uint2color3B(kColorCharcoal));
    //mBgCurtain->setColor(CMUtils::uint2color3B(kColorWhite));
	addChild(mBgCurtain);
#else
    mBgCurtain = CMSprite::createWithTexture(mScene->textureByName("bg-menu"));
    mBgCurtain->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
    //mBgCurtain->setColor(CMUtils::uint2color3B(0));
    
    #ifdef CHEEKY_DESKTOP
    //mBgCurtain->setColor(CMUtils::uint2color3B(0xd5d5d5));
    #endif
    
    addChild(mBgCurtain);
#endif
    
	mDecorations = new Prop();
	mDecorations->init();
	addChild(mDecorations);
    
    mHeaderQuad = CMUtils::createColoredQuad(CCSizeMake(320, 125));
    mHeaderQuad->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() - (mScene->getViewHeight() / 7 + mHeaderQuad->boundingBox().size.height / 2)));
    mHeaderQuad->setColor(CMUtils::uint2color3B(0));
    mHeaderQuad->setVisible(false);
    mDecorations->addChild(mHeaderQuad);
    
    createLevelMenuHeader();
    
    mRateIcon = RateIcon::createWithCategory(-1);
    mRateIcon->setPosition(ccp((contentX - kLevelWidth / 2) - (kLevelPadding + kLevelPadding / 4), contentY + 250));
    mRateIcon->setVisible(false);
    mRateIcon->addEventListener(RateIcon::EV_TYPE_RATE_ICON_PRESSED(), this);
    GameController::GC()->registerForEvent(GameController::EV_TYPE_USER_DID_RATE_THE_GAME(), this);
    mDecorations->addChild(mRateIcon);

    // Footer
    mFooterKey = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("menu-key"));
    mFooterKey->retain();
    
    char labelStr[kLabelReserveLen+1] = ""; memset(labelStr, ' ', kLabelReserveLen * sizeof(char)); labelStr[16] = '\n';
	mFooterLabel = TextUtils::create(labelStr, 44, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
    mFooterLabel->retain();
    mFooterLabel->setString(" ");
	mFooterLabel->setPosition(ccp(
		contentX - 0.45f * mFooterKey->boundingBox().size.width,
        0.17f * mScene->getViewHeight()));
    mFooterLabel->setColor(CMUtils::uint2color3B(0xeceb74));
	mDecorations->addChild(mFooterLabel);
    mDecorations->addChild(mFooterKey);

    mContentShadows = new Prop();
	mContentShadows->init();
    
	mContent = new Prop();
	mContent->init();
    mContent->addChild(mContentShadows);
    
	for (int i = 0; i < (int)mLevels->count(); ++i)
	{
		LevelIcon* levelIcon = new LevelIcon(getCategory(), i, static_cast<Level*>(mLevels->objectAtIndex(i))->getID());
		levelIcon->init();
		//levelIcon->setScaleX(kLevelScale);
		//levelIcon->setLevelTextColor(kColorSlateBlue);
		//levelIcon->setPuzzlesSolvedColor(kColorSlateBlue);
//		levelIcon->setPosition(ccp(
//			-kLevelPadding / 2 + levelIcon->boundingBox().size.width / 2 + (mScene->getViewWidth() - kLevelWidth) / 2 + i * kLevelSeparationX,
//			contentY - 8));
        levelIcon->setScale(kLevelIconWidth / levelIcon->boundingBox().size.width);
        levelIcon->setPosition(ccp(
                                   kLevelPadding + levelIcon->boundingBox().size.width / 2 + (mScene->getViewWidth() - kLevelWidth) / 2 + i * kLevelSeparationX,
                                   contentY - 8));
		mLevelIcons.push_back(levelIcon);
		mContent->addChild(levelIcon);
        
        CCSprite* levelIconShadow = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("shadow-square"));
        levelIconShadow->retain();
        levelIconShadow->setColor(CMUtils::uint2color3B(0));
        levelIconShadow->setScaleX(1.2f * levelIcon->boundingBox().size.width / levelIconShadow->boundingBox().size.width);
        levelIconShadow->setScaleY(1.15f * levelIcon->boundingBox().size.height / levelIconShadow->boundingBox().size.height);
        levelIconShadow->setPosition(ccp(levelIcon->getPositionX() + 12, levelIcon->getPositionY() - 12));
        mLevelIconShadows.push_back(levelIconShadow);
        mContentShadows->addChild(levelIconShadow);
        
        memset(labelStr, ' ', kLabelReserveLen * sizeof(char));
        CCLabelBMFont* levelHeaderLabel = TextUtils::createFX(labelStr, 82, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
        levelHeaderLabel->retain();
        levelHeaderLabel->setString(SceneController::localizeString(kLevelNames[i]));
        levelHeaderLabel->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() - (levelHeaderLabel->boundingBox().size.height / 2 + 36)));
        levelHeaderLabel->setColor(CMUtils::uint2color3B(kLevelColors[i]));
        mLevelHeaders.push_back(levelHeaderLabel);
	}
    
#ifdef CHEEKY_DESKTOP
    mLevelIcons[mLevelIndex]->enableHighlight(true);
#endif

	// Limit content scaling
	mCanvas = new CroppedProp(getCategory(), calcCanvasViewableRegion());
    mCanvas->enableCrop(false);
	//mCanvas->addChild(mContent);
	addChild(mCanvas);
    
    // Arrows
    mPrevPageArrow = MenuButton::create(mScene->spriteFrameByName("level-arrow-prev"), mScene->spriteFrameByName("level-arrow-prev"), this);
	mPrevPageArrow->setPosition(ccp(contentX - (kLevelWidth + mPrevPageArrow->boundingBox().size.width) / 2, contentY - 8));
	//mPrevPageArrow->setScaleX(-1);
	mPrevPageArrow->setTag(kButtonTagPrev);
	mCanvas->addChild(mPrevPageArrow);
    
	mNextPageArrow = MenuButton::create(mScene->spriteFrameByName("level-arrow-next"), mScene->spriteFrameByName("level-arrow-next"), this);
	mNextPageArrow->setPosition(ccp(contentX + (kLevelWidth + mNextPageArrow->boundingBox().size.width) / 2, mPrevPageArrow->getPositionY()));
	mNextPageArrow->setTag(kButtonTagNext);
	mCanvas->addChild(mNextPageArrow);
    
    mCanvas->addChild(mContent); // Place content above arrows on the fake Z-axis.

    float maxScale = calcMaxScale();
	setScale(maxScale);
	float scaleOffsetFactor = maxScale - 1.0f;
	setPosition(-scaleOffsetFactor * mScene->getViewWidth() / 2, -scaleOffsetFactor * mScene->getViewHeight() / 2);
    
    // Level Overlay
    mOverlayContainer = new Prop();
	mOverlayContainer->init();
    //addChild(mOverlayContainer);
    
    mLevelOverlay = new LevelOverlay(getCategory());
    mLevelOverlay->init();
    mLevelOverlay->setPosition(ccp(
                                   mScene->getViewWidth() - 0.6f * mLevelOverlay->getContentBounds().size.width,
                                   mHeaderQuad->getPositionY() - (mLevelOverlay->boundingBox().size.height / 2 + 56)));
    mOverlayContainer->addChild(mLevelOverlay);
    
    // Puzzle Page
	mPuzzlePage = new PuzzlePage();
	mPuzzlePage->init();
	mPuzzlePage->setOpacityChildren(0);
	mPuzzlePage->setVisible(false);
	addChild(mPuzzlePage);

	mBackButton = IconButton::create(TextUtils::FONT_REGULAR, "Back", 46, mScene->spriteFrameByName("back-button-icon"), this, false);
	mBackButton->setTag(kButtonTagBack);
	mPuzzlePage->addChild(mBackButton);

	mScrollTweener = new FloatTweener(0, this, CMTransitions::EASE_OUT);
	mScrollTweener->setTag(kTweenerTagScroll);

	mScene->registerForTouches(CMTouches::TC_MENU, this);

	mContentOpacityTweener = new ByteTweener(0, this, CMTransitions::LINEAR);
	mContentOpacityTweener->setTag(kTweenerTagContentOpacity);
	mDecorationsOpacityTweener = new ByteTweener(0, this, CMTransitions::LINEAR);
	mDecorationsOpacityTweener->setTag(kTweenerTagDecorationsOpacity);
	mBgOpacityTweener = new ByteTweener(0, this, CMTransitions::LINEAR);
	mBgOpacityTweener->setTag(kTweenerTagBgOpacity);
	mPuzzlePageOpacityTweener = new ByteTweener(0, this, CMTransitions::LINEAR);
	mPuzzlePageOpacityTweener->setTag(kTweenerTagPuzzlePageOpacity);


    const int kHelpLabelReserveLen = 64;
    char helpLabelStr[kHelpLabelReserveLen+1] = ""; memset(helpLabelStr, ' ', kHelpLabelReserveLen * sizeof(char));
	mHelpLabel = TextUtils::create(helpLabelStr, 44, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
    mHelpLabel->setString(" ");
	mHelpSprite = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("menu-key"));
	mHelpSprite->setPositionX(mHelpLabel->boundingBox().getMinX() + 348);
	mHelpSprite->setScale(0.8f);
	mHelpContainer = new Prop(-1);
	mHelpContainer->addChild(mHelpLabel);
	mHelpContainer->addChild(mHelpSprite);
	mHelpContainer->setPosition(ccp(
                                    mHeaderQuad->getPositionX(),
                                    mHeaderQuad->getPositionY() - mHeaderQuad->boundingBox().size.height / 2));
	mHelpContainer->setOpacityChildren(0);
	mHelpContainer->setVisible(false);
	mDecorations->addChild(mHelpContainer);
	mHelpOpacityTweener = new ByteTweener(mHelpContainer->getOpacity(), this, CMTransitions::LINEAR);
	mHelpOpacityTweener->setTag(kTweenerTagHelpOpacity);
    
#if CM_BETA
    if (kVersionStr)
    {
        CCLabelBMFont* versionLbl = TextUtils::create(kVersionStr, 32, kCCLabelAutomaticWidth, kCCTextAlignmentLeft, CCPointZero);
        versionLbl->setPosition(ccp(
                                    mScene->getViewWidth() - (10 + versionLbl->boundingBox().size.width / 2),
                                    (BridgingUtility::isPad() ? 0.05f : 0.1f) * mScene->getViewHeight()));
        mDecorations->addChild(versionLbl);
    }
#endif

    mScene->registerResDependent(this);
    mScene->registerLocalizable(this);
    CCDirector::sharedDirector()->getKeypadDispatcher()->addDelegate(this);
    
    
    
    
//    {
//        ParticleProp* fireworks = ParticleProp::createWithCategory(getCategory(), "fireworks.plist", this, true);
//        fireworks->setPosition(ccp(250, 250));
//        addChild(fireworks);
//        mScene->addToJuggler(fireworks);
//    }
//
//    {
//        ParticleProp* fireworks = ParticleProp::createWithCategory(getCategory(), "fireworks.plist", this, true);
//        fireworks->setPosition(ccp(250, 500));
//        fireworks->resetSystem(2.0f);
//        addChild(fireworks);
//        mScene->addToJuggler(fireworks);
//    }
//    
//    {
//        ParticleProp* fireworks = ParticleProp::createWithCategory(getCategory(), "fireworks.plist", this, true);
//        fireworks->setPosition(ccp(500, 250));
//        fireworks->resetSystem(3.5f);
//        addChild(fireworks);
//        mScene->addToJuggler(fireworks);
//    }
    
    
    
    
	return true; // Obvious when it fails
}

void LevelMenu::createLevelMenuHeader(void)
{
    if (mHeaderSprite)
    {
        mHeaderSprite->retain();
        mHeaderSprite->removeFromParent();
        if (mHeaderSprite->getTexture())
            CCTextureCache::sharedTextureCache()->removeTexture(mHeaderSprite->getTexture());
        CC_SAFE_RELEASE_NULL(mHeaderSprite);
    }
    
    mHeaderSprite = LangFX::getLevelMenuHeader();
    
    if (mHeaderSprite && mHeaderQuad)
    {
        //mHeaderSprite->setPosition(ccp(mHeaderQuad->getPositionX(), mHeaderQuad->getPositionY() - 0.4f * mHeaderQuad->boundingBox().size.height));
        mHeaderSprite->setPosition(ccp(mHeaderQuad->getPositionX() + 5, mHeaderQuad->getPositionY())); // +5 because logo shadow "un-centers" the image.
        mDecorations->addChild(mHeaderSprite);
        layoutLevelMenuHeader();
    }
    
//    if (mHeaderSprite == NULL)
//    {
//        mHeaderSprite = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("pw-logo"));
//        mHeaderSprite->setPosition(ccp(mHeaderQuad->getPositionX() + 5, mHeaderQuad->getPositionY())); // +5 because logo shadow "un-centers" the image.
//        mDecorations->addChild(mHeaderSprite);
//    }
}

void LevelMenu::layoutLevelMenuHeader(void)
{
    // Commented: no longer useful with non-vertically-limited logo.
//    if (mHeaderSprite)
//    {
//        mHeaderSprite->setScale(1.0f);
//        CCRect headerBounds = mHeaderSprite->boundingBox();
//        if (headerBounds.size.width != 0 && headerBounds.size.height != 0)
//        {
//            float maxWidth = 0.75f * kLevelWidth, maxHeight = 160;
//#ifdef CHEEKY_DESKTOP
//            CCSize headerSize = LangFX::getLevelMenuHeaderSizeOSX();
//#else
//            CCSize headerSize = headerBounds.size;
//#endif
//            float maxScale = MIN(1.0f, MIN(maxWidth / headerSize.width, maxHeight / headerSize.height));
//            mHeaderSprite->setScale(maxScale);
//            
//            float viewAspect = mScene->getViewAspectRatio();
//            float screenAspect = mScene->getViewportAspectRatio();
//            float deltaAspect = screenAspect - viewAspect;
//            
//            float aspectFactor = 1.0f + MAX(0.0f, (deltaAspect < 0 ? 0.2f : 0.2f - deltaAspect));
//            mHeaderSprite->setScale(aspectFactor * mHeaderSprite->getScale());
//        }
//    }
}

void LevelMenu::onEnter()
{
    refreshArrowVisibility();
    refreshLevelLocks();
    refreshLevelsSolved();
    refreshPuzzlesSolved();
    refreshLevelSpriteVisibility();
    refreshLevelSpriteOpacity();
    resolutionDidChange();
}

// Limit content scaling
float LevelMenu::calcMaxScale(void)
{
    float scaleFactor = 0.8f;
    CCRect limitBounds = CMUtils::unionRect(mFooterLabel->boundingBox(), mHeaderQuad->boundingBox());
    float maxScale = MIN(mScene->getMaximizingContentScaleFactor(), scaleFactor * mScene->getViewHeight() / limitBounds.size.height);
    return maxScale;
}

CCRect LevelMenu::calcCanvasViewableRegion(void)
{
    CCPoint btmLeft;
    btmLeft.x = mLevelIcons[0]->getPosition().x - (mLevelIcons[0]->boundingBox().size.width / 2 + kLevelPadding);
    btmLeft.y = mLevelIcons[0]->getPosition().y - (mLevelIcons[0]->boundingBox().size.height / 2 + kLevelPadding);
    
    CCPoint topRight;
    topRight.x = mLevelIcons[2]->getPosition().x + (mLevelIcons[2]->boundingBox().size.width / 2 + kLevelPadding);
    topRight.y = mLevelIcons[2]->getPosition().y + mLevelIcons[2]->boundingBox().size.height / 2 + kLevelPadding;
    
    btmLeft = convertToWorldSpace(btmLeft);
    topRight = convertToWorldSpace(topRight);
    
    return CCRectMake(
                      btmLeft.x,
                      btmLeft.y,
                      topRight.x - btmLeft.x,
                      topRight.y - btmLeft.y);
}

void LevelMenu::repositionBackButton(void)
{
    if (mPuzzlePage && mBackButton && mBgCurtain)
	{
        mBackButton->setIconOffset(ccp(LangFX::getPuzzleMenuBackButtonIconOffset().val_0,
                                       LangFX::getPuzzleMenuBackButtonIconOffset().val_1));
        
		PuzzlePageEntry* entry = mPuzzlePage->getEntry(PuzzlePage::kNumPuzzlesPerPage-2);
        CCAssert(entry, "LevelMenu::repositionBackButton has a NULL PuzzlePageEntry. This shouldn't be possible.");
        if (entry)
        {
            CCPoint minPoint = entry->getMinPoint();
            minPoint.x -= mBackButton->getIconSize().width / 2;
            CCPoint backPos = entry->convertToWorldSpace(minPoint);
            backPos = this->convertToNodeSpace(backPos);
            float scaledOffset = (getScale() * mBgCurtain->boundingBox().size.height - mBgCurtain->boundingBox().size.height) / 2;
            float yOffset = 0;
            
            LangFX::PuzzlePageSettings settings = PuzzlePage::getSettings();
            Localizer::LocaleType locale = mScene->getLocale();
            switch (locale) {
                case Localizer::DE:
                case Localizer::FR:
                case Localizer::IT:
                    yOffset = -(mBackButton->boundingBox().size.height / 4 + settings.defaultOffsetY / 2);
                    break;
                default:
                    yOffset = -(mBackButton->boundingBox().size.height / 5 + settings.defaultOffsetY / 2);
                    break;
            }
            
            mBackButton->setPosition(ccp(
                                         backPos.x,
                                         (scaledOffset + (backPos.y - scaledOffset) / 2) + yOffset));
        }
	}
}

void LevelMenu::repositionRateIcon(void)
{
    if (mDecorations && mRateIcon)
    {
        mRateIcon->setScale(1.0f / MAX(0.01f, getScaleX()));
        CCSize rateSize = mRateIcon->getContentSize();
        CCPoint nodePos = mDecorations->convertToNodeSpace(ccp(0, mScene->getViewportHeight()));
        mRateIcon->setPosition(ccp(nodePos.x + rateSize.width / 2 + 16, nodePos.y - (rateSize.height / 2 + 10)));
    }
}

void LevelMenu::resolutionDidChange(void)
{
    float maxScale = calcMaxScale();
    setScale(maxScale);
    
    float scaleOffsetFactor = maxScale - 1.0f;
	setPosition(-scaleOffsetFactor * mScene->getViewWidth() / 2, -scaleOffsetFactor * mScene->getViewHeight() / 2);
    
    if (mCanvas)
        mCanvas->setViewableRegion(calcCanvasViewableRegion());
    
    layoutLevelMenuHeader();
    
    if (mPuzzlePage)
    {
        mPuzzlePage->setHeaderLabel(NULL);
        mPuzzlePage->resolutionDidChange(maxScale);
        mPuzzlePage->setHeaderLabel(mLevelHeaders[getLevelIndex()]);
    }
    
    if (mBgCurtain)
    {
        if (maxScale != 0.0f)
            mBgCurtain->setTextureRect(CCRectMake(0, 0, mScene->getFullscreenWidth() * (1.0f / maxScale), mScene->getFullscreenHeight() * (1.0f / maxScale)));
        else
            mBgCurtain->setTextureRect(CCRectMake(0, 0, mScene->getFullscreenWidth(), mScene->getFullscreenHeight()));
        mBgCurtain->setTextureCoordsCM(CCRectMake(0,
                                                  0,
                                                  mScene->getViewportWidth() / CC_CONTENT_SCALE_FACTOR(),
                                                  mScene->getViewportHeight() / CC_CONTENT_SCALE_FACTOR()));
        mBgCurtain->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
    }
    
    repositionRateIcon();
    repositionBackButton();
    refreshLevelSpriteVisibility();
    refreshLevelSpriteOpacity();
}

void LevelMenu::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    if (mFooterLabel)
    {
        mFooterLabel->setString("");
        mFooterLabel->setFntFile(fontKey);
        refreshPuzzlesSolved();
    }
    
    if (mHelpLabel)
    {
        mHelpLabel->setString("");
        mHelpLabel->setFntFile(fontKey);
    }
    
    createLevelMenuHeader();
    
    for (std::vector<LevelIcon*>::iterator it = mLevelIcons.begin(); it != mLevelIcons.end(); ++it)
        (*it)->localeDidChange(fontKey, FXFontKey);
    
    for (std::vector<CCLabelBMFont*>::iterator it = mLevelHeaders.begin(); it != mLevelHeaders.end(); ++it)
        TextUtils::switchFntFile(FXFontKey, *it);
    
    if (mPuzzlePage)
        mPuzzlePage->localeDidChange(fontKey, FXFontKey);
    
    if (getSelectedLevel())
        populatePuzzlePage(getSelectedLevel()->getPuzzles());
    
    if (mRateIcon)
        mRateIcon->localeDidChange(fontKey, FXFontKey);
    
    if (mBackButton)
        mBackButton->localeDidChange(fontKey, FXFontKey);
    
    repositionRateIcon();
    repositionBackButton();
}

void LevelMenu::refresh(void)
{
    refreshPuzzleLocks();
    refreshPuzzlesSolved();
    
    refreshLevelLocks();
    refreshLevelsSolved();
}

void LevelMenu::updateCrashContext(const char* value, const char* key)
{
    if (value && key)
        BridgingUtility::setCrashContext(value, key);
}

void LevelMenu::capFramerate(void)
{
    double fps = MAX(30.0, mScene->getFps());
    LevelMenuState state = getState();
    
    if ((state == LevelMenu::LEVELS || state == LevelMenu::PUZZLES) && !isScrolling() && !mIsTouched)
        CCDirector::sharedDirector()->setAnimationInterval(1.0 / fps); //5.0);
    else
        CCDirector::sharedDirector()->setAnimationInterval(1.0 / fps);
}

void LevelMenu::setState(LevelMenuState value)
{
	if (mState == value)
        return;

	GameProgressController* gpc = GameProgressController::GPC();

    // Clean up previous state
    switch (mState)
    {
		case LevelMenu::IDLE:
            break;
        case LevelMenu::IDLE_2_PUZZLES:
            break;
        case LevelMenu::IDLE_2_LEVELS:
            break;
        case LevelMenu::LEVELS:
            break;
        case LevelMenu::LEVELS_2_PUZZLES:
            break;
        case LevelMenu::PUZZLES:
            break;
        case LevelMenu::PUZZLES_2_LEVELS:
            break;
        case LevelMenu::PUZZLES_2_IDLE:
            break;
    }

    mState = value;

    // Apply new state
    switch (mState)
    {
        case LevelMenu::IDLE:
            mIsTouchEnabled = false;
            setVisible(false);
            break;
        case LevelMenu::IDLE_2_PUZZLES:
            updateCrashContext("None", BridgingUtility::CRASH_CONTEXT_PUZZLE_NAME);
            updateCrashContext(getLevelName(), BridgingUtility::CRASH_CONTEXT_LEVEL_NAME);
            updateCrashContext("PuzzleMenu", BridgingUtility::CRASH_CONTEXT_GAME_STATE);
            
			mIsTouchEnabled = true;
            setVisible(true);
			mPuzzlePage->setVisible(true);
            refreshPuzzleLocks();
            refreshPuzzlesSolved();
            refreshLevelLocks(); // Do them all each time. // LevelIndex+1);
            refreshLevelsSolved(); // Do them all each time. // LevelIndex);
			gpc->save();
            break;
        case LevelMenu::IDLE_2_LEVELS:
            updateCrashContext("None", BridgingUtility::CRASH_CONTEXT_PUZZLE_NAME);
            updateCrashContext("LevelMenu", BridgingUtility::CRASH_CONTEXT_GAME_STATE);
            
            mPuzzlePage->setOpacity(0);
            mPuzzlePageOpacityTweener->reset(mPuzzlePage->getOpacity());
            
            setVisible(true);
            mContent->setVisible(true);
			mDecorations->setVisible(true);
            mOverlayContainer->setVisible(true);
            mHelpContainer->setVisible(false);
            mPuzzlePage->setVisible(false);
            
            refreshPuzzleLocks();
            refreshPuzzlesSolved();
            refreshLevelLocks();
            refreshLevelsSolved();
            gpc->save();
            break;
        case LevelMenu::LEVELS:
        	if (mRateIcon)
        		mRateIcon->reset();
            mPuzzlePage->setVisible(false);
            refreshLevelSpriteOpacity();
            capFramerate();
            break;
        case LevelMenu::LEVELS_2_PUZZLES:
            capFramerate();
            updateCrashContext(getLevelName(), BridgingUtility::CRASH_CONTEXT_LEVEL_NAME);
            updateCrashContext("PuzzleMenu", BridgingUtility::CRASH_CONTEXT_GAME_STATE);
            
            enableBackButtonHighlight(false);
            setPuzzleIndex(0);
			setContentX(-mPageIndex * kScrollDist);
			mPuzzlePage->setHeaderLabel(mLevelHeaders[getLevelIndex()]);
			mPuzzlePage->setHighlightColor(kLevelColors[getLevelIndex()]);
            mBackButton->setSelectedColors(kLevelColors[getLevelIndex()], kLevelColors[getLevelIndex()]);
            mPuzzlePage->setVisible(true);
            refreshPuzzleLocks();
            refreshPuzzlesSolved();
            break;
        case LevelMenu::PUZZLES:
            setIsScrolling(false);
            capFramerate();
			mBackButton->reset();
			mContent->setVisible(false);
			mDecorations->setVisible(false);
            mOverlayContainer->setVisible(false);
			mHelpContainer->setVisible(false);

			if (mQueuedState == LevelMenu::PUZZLES_2_LEVELS)
			{
				mQueuedState = LevelMenu::IDLE;
				transitionPuzzles2Levels();
			}
            break;
		case LevelMenu::PUZZLES_2_LEVELS:
            capFramerate();
            updateCrashContext("LevelMenu", BridgingUtility::CRASH_CONTEXT_GAME_STATE);
            
			mScrollTweener->reset(0);
			setPageIndex(getLevelIndex() / kNumLevelsPerPage);
			setContentX(-mPageIndex * kScrollDist);
            mContent->setVisible(true);
			mDecorations->setVisible(true);
            mOverlayContainer->setVisible(true);
            break;
        case LevelMenu::PUZZLES_2_IDLE:
            capFramerate();
			mIsTouchEnabled = false;
            break;
    }
}

void LevelMenu::setIsScrolling(bool value)
{
    mIsScrolling = value;
    refreshLevelSpriteVisibility();
    refreshLevelSpriteOpacity();
    capFramerate();
}

int LevelMenu::getSelectedPuzzleID(void)
{
    Puzzle* puzzle = getSelectedPuzzle();
	return puzzle ? puzzle->getID() : -1;

}
int LevelMenu::getNumLevels(void) const
{
	return mLevels ? (int)mLevels->count() : 0;
}

Level* LevelMenu::getSelectedLevel(void)
{
	return static_cast<Level*>(mLevels->objectAtIndex(getLevelIndex()));
}

const char* LevelMenu::getLevelName() const
{
    int levelIndex = getLevelIndex();
    if (levelIndex >= 0 && levelIndex < kNumLevels)
        return kLevelNames[levelIndex];
    else
        return NULL;
}

Puzzle* LevelMenu::getSelectedPuzzle(void)
{
    Level* level = getSelectedLevel();
	CCArray* puzzles = level ? level->getPuzzles() : NULL;
	if (puzzles && (int)puzzles->count() > getPuzzleIndex())
		return static_cast<Puzzle*>(puzzles->objectAtIndex(getPuzzleIndex()));
	else
		return NULL;
}

void LevelMenu::setLevelIndex(int value)
{
	if (mLevelIndex != value && value >= 0 && value < (int)mLevelIcons.size())
    {
#ifdef CHEEKY_MOBILE
        mLevelIndex = value;
        refreshArrowVisibility();
#else
        mLevelIcons[mLevelIndex]->enableHighlight(false);
        mLevelIndex = value;
        refreshArrowVisibility();
        mLevelIcons[mLevelIndex]->enableHighlight(true);
        setPageIndex(mLevelIndex / kNumLevelsPerPage);
#endif
    }
}

void LevelMenu::setPuzzleIndex(int value)
{
#ifdef CHEEKY_MOBILE
	mPuzzleIndex = MAX(0, MIN((int)getSelectedLevel()->getPuzzles()->count() - 1, value));
#else
    highlightPuzzle(mPuzzleIndex, false);
    mPuzzleIndex = MAX(0, MIN((int)getSelectedLevel()->getPuzzles()->count() - 1, value));
    
    if (!isBackButtonHighlighted())
        highlightPuzzle(mPuzzleIndex, true);
#endif
}

void LevelMenu::setPageIndex(int value)
{
#ifdef CHEEKY_MOBILE
    mPageIndex = MAX(0, MIN(kNumPages-1, value));
#else
    int prevPageIndex = mPageIndex;
	mPageIndex = MAX(0, MIN(kNumPages-1, value));
    if (prevPageIndex != mPageIndex)
        scrollContentTo(-mPageIndex * kScrollDist);
#endif
}

void LevelMenu::scrollContentBy(float x)
{
	scrollContentTo(mContent->getPositionX() + x);
}

void LevelMenu::scrollContentTo(float x)
{
    float duration = MIN(kLevelScrollDuration, kLevelScrollDuration * mScrollTweener->getTransition()(fabs(x - mContent->getPositionX()) / kScrollDist));
	mScrollTweener->reset(
		mContent->getPositionX(),
		x,
        MAX(0.1f, duration));
    setIsScrolling(true);
}

void LevelMenu::setContentX(float x)
{
	mContent->setPositionX(MIN(0, MAX(x, -(kMaxLevels - kNumLevelsPerPage) * kLevelSeparationX)));
	refreshArrowVisibility();
	refreshLevelSpriteVisibility();
    refreshLevelSpriteOpacity();
}

bool LevelMenu::isLevelUnlocked(int levelIndex)
{
	return getUnlockedAll() || GameProgressController::GPC()->isLevelUnlocked(levelIndex); // Normal
    //return levelIndex == 0 || (!getUnlockedAll() && GameProgressController::GPC()->isLevelUnlocked(levelIndex)); // Opposite: Lock All
    //return levelIndex == 0; // Quick debug lock all
}

bool LevelMenu::isPuzzleUnlocked(int levelIndex, int puzzleIndex)
{
	return getUnlockedAll() || GameProgressController::GPC()->isPuzzleUnlocked(levelIndex, puzzleIndex);
}

bool LevelMenu::isBackButtonHighlighted(void) const
{
    return mBackButton && mBackButton->isSelected();
}

void LevelMenu::enableBackButtonHighlight(bool value)
{
#ifdef CHEEKY_DESKTOP
    if (mBackButton)
        mBackButton->setSelected(value);
#endif
}

void LevelMenu::showHelpUnlock(int levelIndex)
{
	if (levelIndex <= 0 || levelIndex >= kNumLevels || getState() != LevelMenu::LEVELS || mHelpContainer == NULL || mHelpSprite == NULL)
		return;
    
    mHelpContainer->setScale(1.0f);

	std::string msg;

	if (levelIndex == kNumLevels-1)
	{
		msg = CMUtils::strConcatVal(std::string(SceneController::localizeString("Unlock with")).append(" ").c_str(), 66);
        mHelpLabel->setString(msg.c_str());
        mHelpSprite->setPositionX(mHelpLabel->getPositionX() + mHelpLabel->boundingBox().size.width / 2 + 0.5f * mHelpSprite->boundingBox().size.width);
	}
	else
	{
		msg = std::string(SceneController::localizeString("Unlock with")).append(" 3");
        mHelpLabel->setString(msg.c_str());
        float spriteOffsetX = mHelpLabel->boundingBox().size.width + 0.5f * mHelpSprite->boundingBox().size.width;
        
        msg = msg.append(LangFX::getLevelMenuHelpUnlockPaddingString()).append(SceneController::localizeString("in "));
		msg.append(SceneController::localizeString(kLevelNames[levelIndex-1]));
        mHelpLabel->setString(msg.c_str());
        mHelpSprite->setPositionX(mHelpLabel->boundingBox().getMinX() + spriteOffsetX);
	}
    
    mHelpSprite->setPositionY(mHelpLabel->getPositionY() + LangFX::getLevelMenuHelpUnlockYOffsets().val_1);
    
	mHelpLabel->setColor(CMUtils::uint2color3B(kLevelColors[levelIndex]));
    mHelpContainer->setPositionY(mHeaderQuad->getPositionY() - 0.6f * mHeaderQuad->boundingBox().size.height);
	mHelpContainer->setOpacityChildren((GLubyte)(255 * (mDecorations->getOpacity() / 255.0f)));
	mHelpContainer->setVisible(true);
	mHelpOpacityTweener->reset(mHelpContainer->getOpacity(), 0, kHelpFadeDuration, kHelpFadeDelay);
    
    float maxHelpWidth = 700; //630;
    CCRect helpBounds = mHelpLabel->boundingBox();
    if (helpBounds.size.width > maxHelpWidth)
        mHelpContainer->setScale(maxHelpWidth / helpBounds.size.width);
}

void LevelMenu::refreshArrowVisibility(void)
{
	if (mLevelIcons.size() <= kNumLevelsPerPage)
	{
		mPrevPageArrow->enable(false);
		mNextPageArrow->enable(false);
	}
	else
	{
		mPrevPageArrow->enable(mPageIndex != 0);
		mNextPageArrow->enable(mPageIndex != kNumPages-1);
	}
    
    mPrevPageArrow->enforceOpacity(mDecorations->getOpacity());
    mNextPageArrow->enforceOpacity(mDecorations->getOpacity());
}

CCRect LevelMenu::getLevelSpriteViewableRect(LevelIcon* icon, CCRect& iconBounds)
{
    // Project icon bounds to stage coords
    CCPoint btmLeft = icon->convertToWorldSpace(CCPointMake(-iconBounds.size.width / 2, -iconBounds.size.height / 2));
    CCPoint topRight = icon->convertToWorldSpace(CCPointMake(iconBounds.size.width / 2, iconBounds.size.height / 2));
    iconBounds.setRect(
                       btmLeft.x,
                       btmLeft.y,
                       topRight.x - btmLeft.x,
                       topRight.y - btmLeft.y);
    
    // View bounds are already in stage coords
    CCRect viewBounds = mCanvas->getViewableRegion();

    CCRect intersection = CMUtils::intersectionRect(viewBounds, iconBounds);
    return intersection;
}

void LevelMenu::refreshLevelSpriteVisibility(void)
{
    CCAssert(mLevelIcons.size() == mLevelIconShadows.size(), "LevelMenu::refreshLevelSpriteVisibility - invalid state.");
    
	for (int i = 0; i < (int)mLevelIcons.size(); ++i)
	{
        CCRect iconBounds = mLevelIcons[i]->boundingBox();
		CCRect intersection = getLevelSpriteViewableRect(mLevelIcons[i], iconBounds);
		mLevelIcons[i]->setVisible(intersection.size.width > 1.0f && intersection.size.height > 1.0f);
        mLevelIconShadows[i]->setVisible(mLevelIcons[i]->isVisible());
	}
}

void LevelMenu::refreshLevelSpriteOpacity(void)
{
    CCAssert(mLevelIcons.size() == mLevelIconShadows.size(), "LevelMenu::refreshLevelSpriteVisibility - invalid state.");

    GLubyte parentOpacity = mContent->getOpacity();
    for (int i = 0; i < (int)mLevelIcons.size(); ++i)
    {
        CCRect iconBounds = mLevelIcons[i]->boundingBox();
        CCRect intersection = getLevelSpriteViewableRect(mLevelIcons[i], iconBounds);
        GLubyte opacity = parentOpacity * CMTransitions::LINEAR(intersection.size.width <= 1.0f ? 0 : MIN(1.0f, intersection.size.width / iconBounds.size.width));
        mLevelIcons[i]->setOpacityChildren(opacity);
        mLevelIconShadows[i]->setOpacity(opacity);
    }
}

void LevelMenu::refreshLevelLocks(int levelIndex)
{
	if (levelIndex == -1)
    {
		for (int i = 0; i < (int)mLevelIcons.size(); ++i)
			mLevelIcons[i]->setLocked(!isLevelUnlocked(i));
    }
    else if (levelIndex >= 0 && levelIndex < (int)mLevelIcons.size())
		mLevelIcons[levelIndex]->setLocked(!isLevelUnlocked(levelIndex));
}

void LevelMenu::refreshLevelsSolved(int levelIndex)
{
	GameProgressController* gpc = GameProgressController::GPC();
    if (levelIndex == -1)
    {
		for (int i = 0; i < (int)mLevelIcons.size(); ++i)
			mLevelIcons[i]->setPuzzlesSolved(gpc->getNumSolvedPuzzlesForLevel(i), PuzzlePage::kNumPuzzlesPerPage);
    }
	else if (levelIndex >= 0 && levelIndex < (int)mLevelIcons.size())
        mLevelIcons[levelIndex]->setPuzzlesSolved(gpc->getNumSolvedPuzzlesForLevel(levelIndex), PuzzlePage::kNumPuzzlesPerPage);
}

void LevelMenu::refreshPuzzleLocks(void)
{
	for (int i = 0; i < PuzzlePage::kNumPuzzlesPerPage; ++i)
		mPuzzlePage->setLockedAtIndex(i, !isPuzzleUnlocked(getLevelIndex(), i));
}

void LevelMenu::refreshPuzzlesSolved(void)
{
	GameProgressController* gpc = GameProgressController::GPC();
	for (int i = 0; i < PuzzlePage::kNumPuzzlesPerPage; ++i)
		mPuzzlePage->setSolvedAtIndex(i, gpc->hasSolved(getLevelIndex(), i));
    
    Localizer::LocaleType locale = mScene->getLocale();
    std::string spacer = locale == Localizer::CN || locale == Localizer::JP ? "" : " ";
    std::string footerStr = CMUtils::strConcatVal(std::string(SceneController::localizeString("Total Progress:")).append(spacer).c_str(), gpc->getNumSolvedPuzzles());
    footerStr = CMUtils::strConcatVal(footerStr.append("/").c_str(), gpc->getNumPuzzles());
	mFooterLabel->setString(footerStr.c_str());

    mFooterKey->setPosition(ccp(mFooterLabel->getPositionX() + mFooterLabel->getContentSize().width / 2 + LangFX::getLevelMenuTotalProgressOffsets().val_0,
                                mFooterLabel->getPositionY() + LangFX::getLevelMenuTotalProgressOffsets().val_1));
}

void LevelMenu::populatePuzzlePage(CCArray* puzzles)
{
	if (puzzles && mPuzzlePage)
	{
		mPuzzlePage->clear();
		for (int i = 0; i < (int)puzzles->count(); ++i)
			mPuzzlePage->setPuzzleAtIndex(i, static_cast<Puzzle*>(puzzles->objectAtIndex(i)));
	}
}

void LevelMenu::selectCurrentLevel(void)
{
	if (getState() != LevelMenu::LEVELS)
		return;

	if (!isLevelUnlocked(getLevelIndex()))
		mScene->playSound("locked");
    else
    {
		mScene->playSound("button");
		populatePuzzlePage(getSelectedLevel()->getPuzzles());
		transitionLevels2Puzzles();
	}

}

void LevelMenu::selectCurrentPuzzle(void)
{
	if (getState() != LevelMenu::PUZZLES)
        return;

	if (!isPuzzleUnlocked(getLevelIndex(), getPuzzleIndex()))
		mScene->playSound("locked");
    else
    {
		mScene->playSound("button");
		transitionPuzzles2Idle();
		dispatchEvent(this->EV_TYPE_PUZZLE_SELECTED(), this);
	}
}

void LevelMenu::highlightPuzzle(int index, bool enable)
{
	mPuzzlePage->highlightPuzzle(index % PuzzlePage::kNumPuzzlesPerPage, enable);
}

void LevelMenu::enableRateTheGameIcon(bool enable)
{
    if (mRateIcon)
        mRateIcon->setVisible(enable);
}

void LevelMenu::animateRateTheGameIcon(bool enable)
{
    if (mRateIcon)
        mRateIcon->animate(enable);
}

void LevelMenu::refreshColorScheme(void)
{
    if (mPuzzlePage)
        mPuzzlePage->refreshColorScheme();
}

void LevelMenu::jumpToLevelIndex(int levelIndex, int puzzleIndex)
{
	if (mContent == NULL || levelIndex < 0 || levelIndex >= getNumLevels() || puzzleIndex < 0 || puzzleIndex >= PuzzlePage::kNumPuzzlesPerPage)
        return;

    setLevelIndex(levelIndex);
	mScrollTweener->reset(0);
    setIsScrolling(false);
	setPageIndex(getLevelIndex() / kNumLevelsPerPage);
	setContentX(-mPageIndex * kScrollDist);
	populatePuzzlePage(getSelectedLevel()->getPuzzles());
    setPuzzleIndex(puzzleIndex);
	mPuzzlePage->setHeaderLabel(mLevelHeaders[getLevelIndex()]);
	mPuzzlePage->setHighlightColor(kLevelColors[getLevelIndex()]);
    mBackButton->setSelectedColors(kLevelColors[getLevelIndex()], kLevelColors[getLevelIndex()]);
    refreshPuzzleLocks();
    refreshPuzzlesSolved();
    refreshLevelSpriteVisibility();
    refreshLevelSpriteOpacity();
}

void LevelMenu::returnToLevelMenu(void)
{
    if (getState() == LevelMenu::LEVELS)
		dispatchEvent(EV_TYPE_DID_TRANSITION_IN(), this);
    
    if (getState() != LevelMenu::IDLE && getState() != LevelMenu::IDLE_2_PUZZLES)
		return;
    
    float duration = kLevel2PuzzleTransitionDuration + kPuzzle2LevelTransitionDuration;
    setState(LevelMenu::IDLE_2_LEVELS);
    mBgOpacityTweener->reset(mBgCurtain->getOpacity(), 255, ((255 - mBgCurtain->getOpacity()) / (float)255) * duration);
    mContentOpacityTweener->reset(mContent->getOpacity(), 255, ((255 - mContent->getOpacity()) / (float)255) * duration);
	mDecorationsOpacityTweener->reset(mDecorations->getOpacity(), 255, ((255 - mDecorations->getOpacity()) / (float)255) * duration);
    
//	if (getState() != LevelMenu::PUZZLES)
//		mQueuedState = LevelMenu::PUZZLES_2_LEVELS;
//    else
//        transitionPuzzles2Levels();
}

void LevelMenu::showOverTime(float duration)
{
	if (getState() == LevelMenu::LEVELS)
		dispatchEvent(EV_TYPE_DID_TRANSITION_IN(), this);
	if (getState() != LevelMenu::IDLE)
		return;

	setState(LevelMenu::IDLE_2_PUZZLES);
	mBgOpacityTweener->reset(mBgCurtain->getOpacity(), 255, ((255 - mBgCurtain->getOpacity()) / (float)255) * duration);
	mPuzzlePageOpacityTweener->reset(mPuzzlePage->getOpacity(), 255, ((255 - mPuzzlePage->getOpacity()) / (float)255) * duration);
}

void LevelMenu::hideOverTime(float duration)
{
	if (getState() == LevelMenu::IDLE)
		dispatchEvent(EV_TYPE_DID_TRANSITION_OUT(), this);
	if (getState() != LevelMenu::PUZZLES)
		return;

	setState(LevelMenu::PUZZLES_2_IDLE);

	mPuzzlePageOpacityTweener->reset(mPuzzlePage->getOpacity(), 0, 0.4f * (mPuzzlePage->getOpacity() / (float)255) * duration);
	mBgOpacityTweener->reset(mBgCurtain->getOpacity(), 0, (mBgCurtain->getOpacity() / (float)255) * duration);
}

void LevelMenu::hideInstantaneously(void)
{
    mHelpOpacityTweener->reset(0);
    mContentOpacityTweener->reset(0);
	mDecorationsOpacityTweener->reset(0);
	mPuzzlePageOpacityTweener->reset(0);
	mBgOpacityTweener->reset(0);
    
    mContent->setOpacityChildren(0);
    mDecorations->setOpacityChildren(0);
    mLevelOverlay->setOpacity(0);
    mPrevPageArrow->enforceOpacity(0);
    mNextPageArrow->enforceOpacity(0);
    mPuzzlePage->setOpacityChildren(0);
    mBgCurtain->setOpacity(0);
    mHelpContainer->setOpacityChildren(0);
    
    setState(LevelMenu::IDLE);
}

void LevelMenu::transitionLevels2Puzzles(float duration)
{
	if (getState() != LevelMenu::LEVELS)
		return;

	setState(LevelMenu::LEVELS_2_PUZZLES);

	mContent->setOpacityChildren(mContent->getOpacity());
	mDecorations->setOpacityChildren(mDecorations->getOpacity());
	mPuzzlePage->setOpacityChildren(mPuzzlePage->getOpacity());

	mContentOpacityTweener->reset(mContent->getOpacity(), 0, (mContent->getOpacity() / 255) * duration);
	mDecorationsOpacityTweener->reset(mDecorations->getOpacity(), 0, (mDecorations->getOpacity() / (float)255) * duration);
	mPuzzlePageOpacityTweener->reset(mPuzzlePage->getOpacity(), 255, ((255 - mPuzzlePage->getOpacity()) / (float)255) * duration);
}

void LevelMenu::transitionPuzzles2Levels(float duration)
{
	if (getState() != LevelMenu::PUZZLES)
		return;

	setState(LevelMenu::PUZZLES_2_LEVELS);

	mContent->setOpacityChildren(mContent->getOpacity());
	mDecorations->setOpacityChildren(mDecorations->getOpacity());
	mPuzzlePage->setOpacityChildren(mPuzzlePage->getOpacity());

	mContentOpacityTweener->reset(mContent->getOpacity(), 255, ((255 - mContent->getOpacity()) / (float)255) * duration);
	mDecorationsOpacityTweener->reset(mDecorations->getOpacity(), 255, ((255 - mDecorations->getOpacity()) / (float)255) * duration);
	mPuzzlePageOpacityTweener->reset(mPuzzlePage->getOpacity(), 0, (mPuzzlePage->getOpacity() / (float)255) * duration);
}

void LevelMenu::transitionPuzzles2Idle(float duration)
{
	hideOverTime(duration);
}

void LevelMenu::update(int controllerState)
{
#ifndef __ANDROID__
    if (isScrolling() || getState() == LevelMenu::IDLE)
        return;
    
    InputManager* im = InputManager::IM();
    ControlsManager* cm = ControlsManager::CM();
    
    // Poll for input
    bool didSelect = false, didGoBack = false;
    Coord depressedVec = im->getDepressedVector();
    setRepeatVec(depressedVec.x == 0 && depressedVec.y == 0 ? im->getHeldVector() : CMCoordZero);
    
    didSelect = cm->didAccept(true);
    if (!didSelect)
        didGoBack = cm->didKeyDepress(kVK_Delete);
    
    // Process polled input
    if (getState() == LevelMenu::LEVELS)
    {
        if (didSelect)
        {
            if (!isLevelUnlocked(getLevelIndex()))
            {
                showHelpUnlock(getLevelIndex());
                mScene->playSound("locked");
            }
            else
                selectCurrentLevel();
        }
        else
            processNavInput(depressedVec);
    }
    else if (getState() == LevelMenu::PUZZLES)
    {
        if (didSelect)
        {
            if (isBackButtonHighlighted())
                transitionPuzzles2Levels();
            else
                selectCurrentPuzzle();
        }
        else if (didGoBack)
            transitionPuzzles2Levels();
        else
            processNavInput(depressedVec);
    }
#endif
}

void LevelMenu::processNavInput(Coord moveVec)
{
    // Process polled input
    if (getState() == LevelMenu::LEVELS)
    {
        if (moveVec.x == -1)
        {
            if (getLevelIndex() > 0)
                setLevelIndex(getLevelIndex() - 1);
        }
        else if (moveVec.x == 1)
        {
            if (getLevelIndex() < mLevels->count() - 1)
                setLevelIndex(getLevelIndex() + 1);
        }
    }
    else if (getState() == LevelMenu::PUZZLES)
    {
        PuzzlePage* page = mPuzzlePage;
        int row = getPuzzleIndex() / PuzzlePage::kNumPuzzlesPerRow;
        int column = getPuzzleIndex() % PuzzlePage::kNumPuzzlesPerRow;
        
        if (isBackButtonHighlighted())
            moveVec.x = 0;
        
        if (moveVec.x == -1)
        {
            int index = row * PuzzlePage::kNumPuzzlesPerRow + (column + (PuzzlePage::kNumPuzzlesPerRow - 1)) % PuzzlePage::kNumPuzzlesPerRow;
            if (index >= page->getNumPuzzles())
                index = MIN(page->getNumPuzzles() - 1, row * PuzzlePage::kNumPuzzlesPerRow + (PuzzlePage::kNumPuzzlesPerRow - 1));
            setPuzzleIndex(index);
        }
        else if (moveVec.x == 1)
        {
            int index = row * PuzzlePage::kNumPuzzlesPerRow + (column + 1) % PuzzlePage::kNumPuzzlesPerRow;
            if (index >= page->getNumPuzzles())
                index = row * PuzzlePage::kNumPuzzlesPerRow;
            setPuzzleIndex(index);
        }
        else if (moveVec.y != 0)
        {
            if (mBackButton)
            {
                bool wasBackButtonHighlighted = isBackButtonHighlighted();
                enableBackButtonHighlight(false);
                
                if (wasBackButtonHighlighted)
                {
                    int index = (getPuzzleIndex() + PuzzlePage::kNumPuzzlesPerRow) % PuzzlePage::kNumPuzzlesPerPage;
                    if (index >= page->getNumPuzzles())
                        index = getPuzzleIndex();
                    
                    if (moveVec.y == 1)
                        index = index % PuzzlePage::kNumPuzzlesPerRow;
                    else
                        index = index % PuzzlePage::kNumPuzzlesPerRow + PuzzlePage::kNumPuzzlesPerRow;
                    
                    setPuzzleIndex(index);
                }
                else
                {
                    if ((moveVec.y == -1 && getPuzzleIndex() % PuzzlePage::kNumPuzzlesPerPage < PuzzlePage::kNumPuzzlesPerRow)
                        || (moveVec.y == 1 && getPuzzleIndex() % PuzzlePage::kNumPuzzlesPerPage >= PuzzlePage::kNumPuzzlesPerRow))
                    {
                        enableBackButtonHighlight(true);
                        setPuzzleIndex(getPuzzleIndex());
                    }
                    else
                    {
                        int index = (getPuzzleIndex() + PuzzlePage::kNumPuzzlesPerRow) % PuzzlePage::kNumPuzzlesPerPage;
                        if (index >= page->getNumPuzzles())
                            index = getPuzzleIndex();
                        setPuzzleIndex(index);
                    }
                }
            }
            else
            {
                int index = (getPuzzleIndex() + PuzzlePage::kNumPuzzlesPerRow) % PuzzlePage::kNumPuzzlesPerPage;
                if (index >= page->getNumPuzzles())
                    index = getPuzzleIndex();
                setPuzzleIndex(index);
            }
        }
    }
}

void LevelMenu::onTouch(int evType, void* evData)
{
	if (!mIsTouchEnabled)
		return;

	CMTouches::TouchNotice* touchNotice = (CMTouches::TouchNotice*)evData;
	if (touchNotice == NULL)
		return;

	// Pass first touch option to Rate Icon
	bool rateIconHasFocus = false;
	if (mRateIcon && getState() == LevelMenu::LEVELS)
	{
		// Rate icon has focus if it had focus or gains focus
		rateIconHasFocus = mRateIcon->isPressed();
		mRateIcon->onTouch(evType, evData);
		rateIconHasFocus = rateIconHasFocus || mRateIcon->isPressed();
		if (rateIconHasFocus)
			touchNotice->retainFocus(this);
	}

	if (rateIconHasFocus)
		return;

#ifdef CHEEKY_MOBILE
	touchNotice->retainFocus(this);
	CCPoint touchPos = convertToNodeSpace(touchNotice->pos);
    CCPoint prevTouchPos = convertToNodeSpace(touchNotice->prevPos);
    CCPoint deltaPos = ccp(touchPos.x - prevTouchPos.x, touchPos.y - prevTouchPos.y);

	if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN())
	{
		mIsTouched = true;
		mDidTouchMove = mDidTouchCancel = mDidTouchSelect = mIsArrowTouched = false;
		mScrollDir = mScrollDelta = mScrollDeltaAbs = 0;

		switch (getState())
		{
			case LevelMenu::LEVELS:
				{
                    capFramerate();
                    
					// Arrow buttons
					CCRect arrowBounds = CMUtils::boundsInSpace(this, mPrevPageArrow);
					if (arrowBounds.containsPoint(touchPos))
					{
						if (mPrevPageArrow->isEnabled())
							mPrevPageArrow->depress();
						mIsTouched = false;
						mIsArrowTouched = true;
						break;
					}

					arrowBounds = CMUtils::boundsInSpace(this, mNextPageArrow);
					if (arrowBounds.containsPoint(touchPos))
					{
						if (mNextPageArrow->isEnabled())
							mNextPageArrow->depress();
						mIsTouched = false;
						mIsArrowTouched = true;
						break;
					}
                    
                    if (mBgSelectButton)
                    {
                        CCRect bgSelectBounds = CMUtils::boundsInSpace(this, mBgSelectButton);
                        // MenuButton doesn't seem to account for left-alignment.
                        bgSelectBounds.setRect(bgSelectBounds.origin.x - 70,
                                               bgSelectBounds.origin.y,
                                               bgSelectBounds.size.width,
                                               bgSelectBounds.size.height);
                        if (bgSelectBounds.containsPoint(touchPos))
                        {
                            if (mBgSelectButton->isEnabled())
                                mBgSelectButton->depress();
                            mIsTouched = false;
                            mIsArrowTouched = true;
                            break;
                        }
                    }

					mScrollTweener->reset(0);

					// Level icons
					if (mCanvas->getViewableRegion().containsPoint(touchNotice->pos))
					{
                        int iconIndexMin = mPageIndex * kNumLevelsPerPage;
                        int iconIndexMax = iconIndexMin + kNumLevelsPerPage;
						for (int i = iconIndexMin; i < iconIndexMax; ++i)
						{
							CCRect bounds = CMUtils::boundsInSpace(this, mLevelIcons[i]);
							if (bounds.containsPoint(touchPos))
							{
								setLevelIndex(i);
								mLevelIcons[i]->enableHighlight(true);
								mDidTouchSelect = true;
								break;
							}
						}
					}
				}
				break;
			case LevelMenu::PUZZLES:
                {
					CCRect bounds = CMUtils::boundsInSpace(this, mBackButton);
					bounds.setRect(
						bounds.origin.x - bounds.size.width / 8,
						bounds.origin.y - bounds.size.height / 4,
						1.25f * bounds.size.width,
						1.5f * bounds.size.height);
                    if (bounds.containsPoint(touchPos))
                        mBackButton->depress();
                    else
                    {
                        int index = mPuzzlePage->indexAtPoint(touchPos, this);
                        if (index != -1)
                        {
                            setPuzzleIndex(index);
                            highlightPuzzle(mPuzzleIndex, true);
                            mDidTouchSelect = true;
                        }
                    }
                }
                    break;
            default:
                break;
		}
	}
	else if (evType == TouchPad::EV_TYPE_TOUCH_ENDED() || evType == TouchPad::EV_TYPE_TOUCH_CANCELLED())
	{
		mIsTouched = false;
		switch (getState())
		{
			case LevelMenu::LEVELS:
				{
					mLevelIcons[getLevelIndex()]->enableHighlight(false);

					if (mIsArrowTouched)
					{
						if (mPrevPageArrow->isPressed())
						{
							mScrollDir = 1;
							setPageIndex(mPageIndex-1);
							mPrevPageArrow->raise();
						}
						else if (mNextPageArrow->isPressed())
						{
							mScrollDir = -1;
							setPageIndex(mPageIndex+1);
							mNextPageArrow->raise();
						}
                        else if (mBgSelectButton && mBgSelectButton->isPressed())
                        {
                            mBgSelectButton->raise();
                        }
					}
					else if ((!mDidTouchMove || mScrollDeltaAbs <= kMovedTouchThreshold) && !mDidTouchCancel && mDidTouchSelect && isLevelUnlocked(getLevelIndex()))
					{
						selectCurrentLevel();
					}
					else
					{
						if (!mDidTouchMove && !mDidTouchCancel && mDidTouchSelect && !isLevelUnlocked(getLevelIndex())) // Hack
						{
							showHelpUnlock(getLevelIndex());
							mScene->playSound("locked");
						}

                        if (mScrollDelta != 0)
                        {
                            int pageDelta = fabs(mScrollDelta) > kLevelWidth ? 2 : 1;
                            int pageIndex = mPageIndex;
                            if (mDidTouchMove && abs(mScrollDir) > kScrollTouchThreshold)
                            {
                                pageIndex = mScrollDir > 0 ? mPageIndex - pageDelta : mPageIndex + pageDelta;
                                //CCLog("Scrolling %s to mPageIndex:%d with mScrollDir:%d", mScrollDir > 0 ? "LEFT" : "RIGHT", pageIndex, mScrollDir);
                            }
//                            else
//                            {
//                                //pageIndex = abs((int)(mContent->getPositionX() + kLevelWidth / 2) / (kLevelWidth-1));
//                                //CCLog("Scrolling NOWHERE to mPageIndex:%d with mScrollDir:%d", pageIndex, mScrollDir);
//                            }
                            
                            setPageIndex(pageIndex);
                        }
                        
                        scrollContentTo(-mPageIndex * kScrollDist);
					}

					mPrevPageArrow->reset();
					mNextPageArrow->reset();
                    if (mBgSelectButton)
                        mBgSelectButton->reset();
                    capFramerate();
				}
				break;
			case LevelMenu::PUZZLES:
				{
					if (evType == TouchPad::EV_TYPE_TOUCH_ENDED())
						mBackButton->raise();
					else
						mBackButton->reset();

					if (mDidTouchSelect)
					{
						highlightPuzzle(mPuzzleIndex, false);
						if (evType == TouchPad::EV_TYPE_TOUCH_ENDED())
							selectCurrentPuzzle();
					}
				}
				break;
            default:
                break;
		}
	}
	else if (evType == TouchPad::EV_TYPE_TOUCH_MOVED())
	{
		switch (getState())
		{
			case LevelMenu::LEVELS:
				{
					if (mIsArrowTouched)
						break;

                    float deltaX = deltaPos.x;
					mScrollDelta += deltaX;
					mScrollDeltaAbs += fabs(deltaX);
                    
                    do
                    {
                        int levelIndex = getLevelIndex();
                        if (mScrollDeltaAbs <= kMovedTouchThreshold)
                        {
                            if (mDidTouchSelect)
                            {
                                // Check y pos
                                CCRect cancelBounds = CMUtils::boundsInSpace(this, mLevelIcons[levelIndex]);
                                cancelBounds.setRect(
                                                     cancelBounds.origin.x - 20,
                                                     cancelBounds.origin.y - 20,
                                                     cancelBounds.size.width + 2 * 20,
                                                     cancelBounds.size.height + 2 * 20);
                                if (!cancelBounds.containsPoint(touchPos))
                                    mDidTouchCancel = true;
                                else
                                    break;
                            }
                            else
                                break;
                        }
                        
                        mDidTouchMove = true;
						mLevelIcons[levelIndex]->enableHighlight(false);
                        setContentX(mContent->getPositionX() + deltaX);
                    } while (false);

					if (deltaX > 0 && mScrollDir <= 0)
					{
						//setPageIndex(abs((int)mContent->getPositionX() / (kLevelWidth-1)));
						mScrollDir = MAX(1, (int)deltaX);
					}
					else if (deltaX < 0 && mScrollDir >= 0)
					{
						//setPageIndex(abs((int)mContent->getPositionX() / (kLevelWidth-1)));
						mScrollDir = MIN(-1, (int)deltaX);
					}
					else if (deltaX > 0)
						mScrollDir += MAX(1, (int)deltaX);
					else if (deltaX < 0)
						mScrollDir += MIN(-1, (int)deltaX);
				}
				break;
			case LevelMenu::PUZZLES:
				{
					CCRect bounds = CMUtils::boundsInSpace(this, mBackButton);
					bounds.setRect(
						bounds.origin.x - bounds.size.width / 2,
						bounds.origin.y - bounds.size.height,
						2 * bounds.size.width,
						4 * bounds.size.height);
					if (!bounds.containsPoint(touchPos))
						mBackButton->reset();

					if (mDidTouchSelect)
					{
						int index = mPuzzlePage->indexAtPoint(touchPos, this);
						if (index == -1)
						{
							highlightPuzzle(mPuzzleIndex, false);
							mDidTouchSelect = false;
						}
					}
				}
				break;
            default:
                break;
		}
	}
#endif
}

void LevelMenu::keyBackClicked()
{
#ifdef __ANDROID__
	switch (getState())
	{
		case LevelMenu::PUZZLES:
			transitionPuzzles2Levels();
			break;
		default:
			dispatchEvent(EV_TYPE_DEVICE_BACK_CLICKED(), this);
	}
#endif
}

void LevelMenu::onEvent(int evType, void* evData)
{
	if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN() || evType == TouchPad::EV_TYPE_TOUCH_MOVED() ||
		evType == TouchPad::EV_TYPE_TOUCH_ENDED() || evType == TouchPad::EV_TYPE_TOUCH_CANCELLED())
	{
		onTouch(evType, evData);
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			float tweenedValue = tweener->getTweenedValue();
			switch (tweener->getTag())
			{
				case kTweenerTagScroll:
					if (!mIsTouched)
						setContentX(tweenedValue);
					break;
			}
		}
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			switch (tweener->getTag())
			{
				case kTweenerTagScroll:
                    setIsScrolling(false);
					break;
			}
		}
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
		{
			GLubyte tweenedValue = tweener->getTweenedValue();
			switch (tweener->getTag())
			{
				case kTweenerTagContentOpacity:
					mContent->setOpacityChildren(tweenedValue);
					break;
				case kTweenerTagDecorationsOpacity:
					mDecorations->setOpacityChildren(tweenedValue);
                    mLevelOverlay->setOpacity(tweenedValue);
					mPrevPageArrow->enforceOpacity(tweenedValue);
					mNextPageArrow->enforceOpacity(tweenedValue);
					break;
				case kTweenerTagPuzzlePageOpacity:
					mPuzzlePage->setOpacityChildren(tweenedValue);
					break;
				case kTweenerTagBgOpacity:
					mBgCurtain->setOpacity(tweenedValue);
					break;
				case kTweenerTagHelpOpacity:
					mHelpContainer->setOpacityChildren((GLubyte)(tweenedValue * (mDecorations->getOpacity() / 255.0f)));
					break;
			}
		}
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
		{
			switch (tweener->getTag())
			{
				case kTweenerTagBgOpacity:
					{
						if (getState() == LevelMenu::IDLE_2_PUZZLES)
						{
							setState(LevelMenu::PUZZLES);
							dispatchEvent(EV_TYPE_DID_TRANSITION_IN(), this);
						}
                        else if (getState() == LevelMenu::IDLE_2_LEVELS)
                        {
                            setState(LevelMenu::LEVELS);
                            dispatchEvent(EV_TYPE_DID_TRANSITION_IN(), this);
                        }
						else if (getState() == LevelMenu::PUZZLES_2_IDLE)
						{
							setState(LevelMenu::IDLE);
							dispatchEvent(EV_TYPE_DID_TRANSITION_OUT(), this);
						}
					}
					break;
				case kTweenerTagPuzzlePageOpacity:
					{
						if (getState() == LevelMenu::LEVELS_2_PUZZLES)
							setState(LevelMenu::PUZZLES);
						else if (getState() == LevelMenu::PUZZLES_2_LEVELS)
							setState(LevelMenu::LEVELS);
					}
					break;
				case kTweenerTagHelpOpacity:
					mHelpContainer->setVisible(false);
					break;
			}
		}
	}
	else if (evType == MenuButton::EV_TYPE_RAISED())
	{
		MenuButton* button = static_cast<MenuButton*>(evData);
		if (button)
		{
			switch (button->getTag())
			{
				case kButtonTagPrev:
                {
                    if (getState() == LevelMenu::LEVELS)
                        scrollContentTo(-mPageIndex * kScrollDist);
                }
					break;
				case kButtonTagNext:
                {
                    if (getState() == LevelMenu::LEVELS)
                        scrollContentTo(-mPageIndex * kScrollDist);
                }
					break;
				case kButtonTagBack:
                {
                    if (getState() == LevelMenu::PUZZLES)
                        transitionPuzzles2Levels();
                }
					break;
			}
		}
	}
    else if (evType == RateIcon::EV_TYPE_RATE_ICON_PRESSED())
    {
        animateRateTheGameIcon(true);
        dispatchEvent(EV_TYPE_RATE_THE_GAME(), this);
    }
    else if (evType == ParticleProp::EV_TYPE_DID_COMPLETE())
    {
        ParticleProp* fireworks = static_cast<ParticleProp*>(evData);
        if (fireworks)
        {
            float xRange = mScene->getViewWidth(), yRange = mScene->getViewHeight();
            fireworks->setPosition(ccp(
                                       CMUtils::nextRandom(0.15f * xRange, 0.85f * xRange),
                                       CMUtils::nextRandom(0.15f * yRange, 0.85f * yRange)));
            fireworks->resetSystem(1.0f);
            mScene->playSound("fireworks");
        }
    }
    else if (evType == GameController::EV_TYPE_USER_DID_RATE_THE_GAME())
    {
        // If evData is NULL, leave the user alone.
        bool didRateSucceed = evData ? *(bool*)evData : true;
        enableRateTheGameIcon(!didRateSucceed);
        animateRateTheGameIcon(false);
        BridgingUtility::userDidRateTheGame(didRateSucceed);
    }
}

void LevelMenu::advanceTime(float dt)
{
	if (!mIsTouched)
		mScrollTweener->advanceTime(dt);
    
    if (mIsTouched || isScrolling())
        refreshLevelSpriteOpacity();
	
    //mLevelOverlay->advanceTime(dt); // From 0.7 this is no longer visible, so don't bother advancing it.
	mContentOpacityTweener->advanceTime(dt);
	mDecorationsOpacityTweener->advanceTime(dt);
	mPuzzlePageOpacityTweener->advanceTime(dt);
	mBgOpacityTweener->advanceTime(dt);
	mHelpOpacityTweener->advanceTime(dt);

    if (mRateIcon)
        mRateIcon->advanceTime(dt);
    
    
#ifdef CHEEKY_DESKTOP
    if (!isScrolling())
    {
        Coord repeatVec = getRepeatVec();
        if (repeatVec.x != 0 || repeatVec.y != 0)
        {
            mRepeatCounter -= dt;
            if (mRepeatCounter <= 0)
            {
                mRepeatCounter = mRepeatDelay;
                processNavInput(repeatVec);
            }
        }
    }
#endif
}
