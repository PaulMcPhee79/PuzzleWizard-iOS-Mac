
#include "MenuCustomDialog.h"
#include <Puzzle/View/Menu/LevelMenu.h>
#include <Prop/CroppedProp.h>
#include <Managers/InputManager.h>
#include <Puzzle/Inputs/TouchPad.h>
#include <Puzzle/View/Menu/PuzzlePage.h>
#include <Puzzle/View/Menu/LevelIcon.h>
#include <Utils/FloatTweener.h>
#include <Utils/ByteTweener.h>
#include <Prop/ParticleProp.h>
#include <Events/DelayedEvent.h>
#include <Utils/Utils.h>
#include <Utils/LangFX.h>
USING_NS_CC;

//  Misc
static const float kPI = 3.14159265f;

// CustomDialogType::LEVEL_UNLOCKED
static const float kLevelUnlockedWidthMax = 800.0f; // Hack note: linked to MenuBuilder bg width
static const float kSolvedKeyTweenDuration = 0.65f;
static const float kSolvedKeyTweenDelay = 0.5f;
static const int kSolvedKeyCount = 3;
static const int kLevelUnlockedTagX = 1000;
static const int kLevelUnlockedTagY = 2000;
static const int kLevelUnlockedTagRotation = 3000;
static const int kLevelUnlockedTagScale = 4000;
static const int kLevelUnlockedTagArrowAlpha = 5000;
static const int kLevelUnlockedTagArrowX = 5001;
static const int kLevelUnlockedTagLockDrop = 6000;
static const int kLevelUnlockedTagGlow = 7000;

// CustomDialogType::WIZARD_UNLOCKED
static const int kWizardKeyCount = 66;

// CustomDialogType::LEVEL_COMPLETE
static const float kLevelHeaderWidthMax = 500.0f;

// CustomDIalogType::WIZARD
static const float kStarEllipseRadiusX = 124.0f;
static const float kStarEllipseRadiusY = 130.0f;
static const int kDelayedEvPlayCheerSound = 0x1001;
static const int kDelayedEvDropConfetti = 0x1002;
static const int kDelayedEvLaunchFireworks = 0x1003;

// CustomDIalogType::CREDITS
#ifdef CHEEKY_MOBILE
    static const float kDefaultScrollRate = 2.0f;
    static const float kMaxScrollRate = 25.0f;
    static const float kScrollRateDecelFactor = 0.925f;
#else
    static const float kDefaultScrollRate = 1.0f;
    static const float kMaxScrollRate = 5.0f;
    static const float kScrollRateDecelFactor = 0.9f;
#endif

#pragma warning( disable : 4351 ) // new behavior: elements of array 'MenuCustomDialog::mTouchTrail' will be default initialized (changed in Visual C++ 2005) 
MenuCustomDialog::MenuCustomDialog(int category, int priority, int levelIndex, uint inputFocus, CustomDialogType type)
	:
MenuDialog(category, priority, inputFocus, NAV_VERT),
mType(type),
mLevelIndex(levelIndex),
rndToggle(false),
mStarAngle(0),
mStarsContainer(NULL),
mConfetti(NULL),
mFireworks(NULL),
mLevelHeader(NULL),
localeCache(Localizer::INVALID_LOCALE),
mIsTouched(false),
mTrailIndex(0),
mTouchDelta(0),
mTouchTrail(),
mScrollRate(kDefaultScrollRate),
mScrollRateDelta(0),
mScrollContentBounds(CCRectZero),
mScrollBounds(CCRectZero),
mScrollContainer(NULL),
mScrollCropper(NULL),
mSolvedCount(0),
mSolvedGlowDir(1),
mLevelArrow(NULL),
mLevelLock(NULL),
mLevelUnlocked(NULL),
mSolvedKeys(NULL),
mLevelIcons(NULL),
mTweenersX(NULL),
mTweenersY(NULL),
mTweenersRotation(NULL),
mTweenersScale(NULL),
mTweenerArrowAlpha(NULL),
mTweenerArrowX(NULL),
mTweenerLockY(NULL),
mTweenerGlow(NULL),
mTotalSolvedCount(0),
mDropLockFrameCountdown(0),
mTotalSolvedKey(NULL),
mTotalSolvedLabel(NULL)
{
	mAdvanceable = true;
    
    int minLevelindex = mType == MenuCustomDialog::LEVEL_UNLOCKED ? 1 : 0;
    if (mLevelIndex >= LevelMenu::kNumLevels)
        mLevelIndex = LevelMenu::kNumLevels-1;
    if (mLevelIndex < minLevelindex)
		mLevelIndex = minLevelindex;

	for (int i = 0; i < 3; ++i)
		mRGBIncrements[i] = 1;

	mRGB[0] = 0;
	mRGB[1] = 128;
	mRGB[2] = 255;
}

MenuCustomDialog::~MenuCustomDialog(void)
{
    mScene->removeTweensWithTarget(static_cast<IEventListener*>(this));
    
    if (mSolvedKeys)
    {
        CMUtils::release_them(*mSolvedKeys);
        CC_SAFE_DELETE(mSolvedKeys);
        mSolvedKeys = NULL;
    }
    
    if (mLevelIcons)
    {
        CMUtils::release_them(*mLevelIcons);
        CC_SAFE_DELETE(mLevelIcons);
        mLevelIcons = NULL;
    }
    
    if (mTweenersX)
    {
        CMUtils::release_them(*mTweenersX);
        CC_SAFE_DELETE(mTweenersX);
        mTweenersX = NULL;
    }
    
    if (mTweenersY)
    {
        CMUtils::release_them(*mTweenersY);
        CC_SAFE_DELETE(mTweenersY);
        mTweenersY = NULL;
    }
    
    if (mTweenersRotation)
    {
        CMUtils::release_them(*mTweenersRotation);
        CC_SAFE_DELETE(mTweenersRotation);
        mTweenersRotation = NULL;
    }
    
    if (mTweenersScale)
    {
        CMUtils::release_them(*mTweenersScale);
        CC_SAFE_DELETE(mTweenersScale);
        mTweenersScale = NULL;
    }
    
    if (mFireworks)
    {
        for (std::vector<ParticleProp*>::iterator it = mFireworks->begin(); it != mFireworks->end(); ++it)
        {
            ParticleProp* firework = *it;
            if (firework)
            {
                mScene->removeFromJuggler(firework);
                CC_SAFE_RELEASE(firework);
            }
        }
        
        CC_SAFE_DELETE(mFireworks);
        mFireworks = NULL;
    }
    
    for (std::vector<unsigned int>::iterator it = mFireworkSoundIds.begin(); it != mFireworkSoundIds.end(); ++it)
        mScene->stopSound(*it);
    
    CC_SAFE_RELEASE_NULL(mConfetti);
    CC_SAFE_RELEASE_NULL(mLevelArrow);
    CC_SAFE_RELEASE_NULL(mLevelLock);
    CC_SAFE_RELEASE_NULL(mLevelUnlocked);
    CC_SAFE_RELEASE_NULL(mTweenerArrowAlpha);
    CC_SAFE_RELEASE_NULL(mTweenerArrowX);
    CC_SAFE_RELEASE_NULL(mTweenerLockY);
    CC_SAFE_RELEASE_NULL(mTweenerGlow);
    CC_SAFE_RELEASE_NULL(mTotalSolvedKey);
    CC_SAFE_RELEASE_NULL(mTotalSolvedLabel);
	CC_SAFE_RELEASE_NULL(mStarsContainer);
    CC_SAFE_RELEASE_NULL(mScrollContainer);
    CC_SAFE_RELEASE_NULL(mScrollCropper);
}

MenuCustomDialog* MenuCustomDialog::create(int category, int priority, int levelIndex, uint inputFocus, IEventListener* listener, CustomDialogType type, bool autorelease)
{
	MenuCustomDialog *dialog = new MenuCustomDialog(category, priority, levelIndex, inputFocus, type);
	if (dialog && dialog->init())
    {
		dialog->setListener(listener);
		if (autorelease)
			dialog->autorelease();
        return dialog;
    }
    CC_SAFE_DELETE(dialog);
    return NULL;
}

bool MenuCustomDialog::init(void)
{
	bool retVal = MenuDialog::init();

	if (retVal)
	{
		switch (mType)
		{
            case MenuCustomDialog::LEVEL_UNLOCKED:
                {
                    mSolvedKeys = new std::vector<CCSprite*>();
                    mLevelIcons = new std::vector<LevelIcon*>();
                    mTweenersX = new std::vector<FloatTweener*>();
                    mTweenersY = new std::vector<FloatTweener*>();
                    mTweenersRotation = new std::vector<FloatTweener*>();
                    mTweenersScale = new std::vector<FloatTweener*>();
                    
                    CCNode* containerNode = new CCNode();
                    addChild(containerNode, kZOrderContent + 1);
                    containerNode->autorelease();
                    
                    mLevelArrow = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("level-arrow-next"));
                    mLevelArrow->retain();
                    mLevelArrow->setPosition(ccp(0, -24));
                    mLevelArrow->setOpacity(0);
                    mLevelArrow->setVisible(false);
                    
                    for (int i = 0; i < 2; ++i)
                    {
                        LevelIcon* levelIcon = new LevelIcon(-1, mLevelIndex + (i - 1), -1);
                        levelIcon->init();
                        levelIcon->setPosition(ccp(
                                                   i == 0
                                                   ? mLevelArrow->getPositionX() - 0.45f * (mLevelArrow->boundingBox().size.width + levelIcon->boundingBox().size.width)
                                                   : mLevelArrow->getPositionX() + 0.45f * (mLevelArrow->boundingBox().size.width + levelIcon->boundingBox().size.width),
                                                   mLevelArrow->getPositionY()
                                               ));
                        //levelIcon->setPuzzlesSolvedColor(0x6a5acd); // Slate blue
                        levelIcon->setLocked(false);
                        levelIcon->enableSolvedGlow(true);
                        mLevelIcons->push_back(levelIcon);
                        containerNode->addChild(levelIcon);
                    }
                    
                    mLevelLock = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("level-lock"));
                    mLevelLock->retain();
                    
                    CCPoint levelIconPos = containerNode->convertToNodeSpace((*mLevelIcons)[1]->getVisibleWorldCenter());
                    CCRect levelIconBounds = (*mLevelIcons)[1]->getVisibleBounds();
                    mLevelLock->setPosition(ccp(
                                                levelIconPos.x,
                                                levelIconPos.y - (levelIconBounds.size.height - 0.875f * mLevelLock->boundingBox().size.height) / 2));
                    containerNode->addChild(mLevelLock);
                    
                    mLevelUnlocked = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("level-unlocked"));
                    mLevelUnlocked->retain();
                    mLevelUnlocked->setPosition(ccp(mLevelLock->getPositionX(), mLevelLock->getPositionY()));
                    mLevelUnlocked->setVisible(false);
                    containerNode->addChild(mLevelUnlocked);
                    
                    float solvedKeyX = 0, solvedKeyY = 0;
                    CCPoint keyCenter = (*mLevelIcons)[0]->getSolvedKeyWorldCenter();
                    keyCenter = containerNode->convertToNodeSpace(keyCenter);
                    
                    for (int i = 0; i < kSolvedKeyCount; ++i)
                    {
                        CCSprite* solvedKeySprite = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("menu-key"));
                        solvedKeySprite->retain();
                        
                        if (i == 0)
                        {
                            solvedKeyX = mLevelArrow->getPositionX() - 1.25f * solvedKeySprite->boundingBox().size.width;
                            solvedKeyY = (mLevelLock->getPositionY() - mLevelLock->boundingBox().size.height / 2)
                                            + (*mLevelIcons)[0]->getVisibleBounds().size.height + solvedKeySprite->boundingBox().size.height / 2;
                        }
                        else
                        {
                            solvedKeyX += 1.25f * solvedKeySprite->boundingBox().size.width;
                        }
                        
                        solvedKeySprite->setPosition(ccp(solvedKeyX, solvedKeyY));
                        mSolvedKeys->push_back(solvedKeySprite);
                        containerNode->addChild(solvedKeySprite);
                        
                        FloatTweener* tweenerX = new FloatTweener(0, this, CMTransitions::LINEAR);
                        tweenerX->setTag(kLevelUnlockedTagX + i);
                        tweenerX->reset(solvedKeySprite->getPositionX(),
                                        keyCenter.x,
                                        kSolvedKeyTweenDuration,
                                        (i + 2) * kSolvedKeyTweenDelay);
                        mTweenersX->push_back(tweenerX);
                        
                        FloatTweener* tweenerY = new FloatTweener(0, this, CMTransitions::LINEAR);
                        tweenerY->setTag(kLevelUnlockedTagY + i);
                        tweenerY->reset(solvedKeySprite->getPositionY(),
                                        keyCenter.y,
                                        kSolvedKeyTweenDuration,
                                        (i + 2) * kSolvedKeyTweenDelay);
                        mTweenersY->push_back(tweenerY);
                        
                        FloatTweener* tweenerRotation = new FloatTweener(0, this, CMTransitions::LINEAR);
                        tweenerRotation->setTag(kLevelUnlockedTagRotation + i);
                        tweenerRotation->reset(solvedKeySprite->getRotation(),
                                        (*mLevelIcons)[0]->getRotation() - 360,
                                        kSolvedKeyTweenDuration,
                                        (i + 2) * kSolvedKeyTweenDelay);
                        mTweenersRotation->push_back(tweenerRotation);
                        
                        FloatTweener* tweenerScale = new FloatTweener(0, this, CMTransitions::LINEAR);
                        tweenerScale->setTag(kLevelUnlockedTagScale + i);
                        tweenerScale->reset(solvedKeySprite->getScale(),
                                               0.7f * (*mLevelIcons)[0]->getScale(),
                                               kSolvedKeyTweenDuration,
                                               (i + 2) * kSolvedKeyTweenDelay);
                        mTweenersScale->push_back(tweenerScale);
                    }
                    
                    containerNode->addChild(mLevelArrow);
                }
                break;
            case MenuCustomDialog::WIZARD_UNLOCKED:
                {
                    mSolvedKeys = new std::vector<CCSprite*>();
                    mSolvedKeys->reserve(kWizardKeyCount);
                    mLevelIcons = new std::vector<LevelIcon*>();
                    mTweenersX = new std::vector<FloatTweener*>();
                    mTweenersX->reserve(kWizardKeyCount);
                    mTweenersY = new std::vector<FloatTweener*>();
                    mTweenersY->reserve(kWizardKeyCount);
                    mTweenersRotation = new std::vector<FloatTweener*>();
                    mTweenersRotation->reserve(kWizardKeyCount);
                    mTweenersScale = new std::vector<FloatTweener*>();
                    mTweenersScale->reserve(kWizardKeyCount);
                    
                    CCNode* containerNode = new CCNode();
                    addChild(containerNode, kZOrderContent + 1);
                    containerNode->autorelease();
                    
                    LevelIcon* levelIcon = new LevelIcon(-1, mLevelIndex, -1);
                    levelIcon->init();
                    levelIcon->setPosition(ccp(0, 12));
                    //levelIcon->setPuzzlesSolvedColor(0x6a5acd); // Slate blue
                    levelIcon->setLocked(false);
                    mLevelIcons->push_back(levelIcon);
                    containerNode->addChild(levelIcon);
                    
                    mLevelLock = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("level-lock"));
                    mLevelLock->retain();
                    
                    CCPoint levelIconPos = containerNode->convertToNodeSpace((*mLevelIcons)[0]->getVisibleWorldCenter());
                    CCRect levelIconBounds = (*mLevelIcons)[0]->getVisibleBounds();
                    mLevelLock->setPosition(ccp(
                                                levelIconPos.x,
                                                levelIconPos.y - (levelIconBounds.size.height - 0.875f * mLevelLock->boundingBox().size.height) / 2));
                    containerNode->addChild(mLevelLock);
                    
                    mLevelUnlocked = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("level-unlocked"));
                    mLevelUnlocked->retain();
                    mLevelUnlocked->setPosition(ccp(mLevelLock->getPositionX(), mLevelLock->getPositionY()));
                    mLevelUnlocked->setVisible(false);
                    containerNode->addChild(mLevelUnlocked);
                    
                    mTotalSolvedLabel = TextUtils::create(
                                                          CMUtils::strConcatVal(CMUtils::strConcatVal("", kWizardKeyCount).append("/").c_str(), kWizardKeyCount).c_str(),
                                                          48,
                                                          kCCLabelAutomaticWidth,
                                                          kCCTextAlignmentRight,
                                                          CCPointZero);
                    mTotalSolvedLabel->retain();
                    mTotalSolvedLabel->setAnchorPoint(ccp(1.0f, 0.5f));
                    mTotalSolvedLabel->setPosition(ccp(
                                                  levelIconPos.x + mTotalSolvedLabel->boundingBox().size.width / 2,
                                                  levelIconPos.y - (levelIconBounds.size.height / 2 + 0.6f * mTotalSolvedLabel->boundingBox().size.height)));
                    containerNode->addChild(mTotalSolvedLabel);
                    mTotalSolvedLabel->setString(CMUtils::strConcatVal("0/", kWizardKeyCount).c_str());
                    
                    mTotalSolvedKey = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("menu-key"));
                    mTotalSolvedKey->retain();
                    mTotalSolvedKey->setPosition(ccp(
                                                     mTotalSolvedLabel->getPositionX() + 0.45f * mTotalSolvedKey->boundingBox().size.width,
                                                     mTotalSolvedLabel->getPositionY() + LangFX::getCustomDialogKeyYOffset()));
                    containerNode->addChild(mTotalSolvedKey);
                    
                    float solvedKeyScale = 0.5f;
                    float solvedKeyX_0 = levelIconPos.x - (levelIconBounds.size.width / 2 + 2.25f * mTotalSolvedKey->boundingBox().size.width);
                    float solvedKeyX_1 = levelIconPos.x + (levelIconBounds.size.width / 2 + 0.75f * mTotalSolvedKey->boundingBox().size.width);
                    float solvedKeyX = solvedKeyX_0;
                    float solvedKeyY = levelIconPos.y + levelIconBounds.size.height / 2 + solvedKeyScale * mTotalSolvedKey->boundingBox().size.height - 12;
                    for (int i = 0; i < kWizardKeyCount; ++i)
                    {
                        if (i < kWizardKeyCount / 2)
                        {
                            if ((i <= 28 && i % 4 == 0) || i == 31)
                            {
                                solvedKeyX = solvedKeyX_0;
                                solvedKeyY = solvedKeyY - solvedKeyScale * mTotalSolvedKey->boundingBox().size.height;
                            }
                        }
                        else if (i == kWizardKeyCount / 2)
                        {
                            solvedKeyX = solvedKeyX_1;
                            solvedKeyY = levelIconPos.y + levelIconBounds.size.height / 2 - 12;
                        }
                        else
                        {
                            int iMod = i % (kWizardKeyCount / 2);
                            if (iMod < 28 && iMod % 4 == 0)
                            {
                                solvedKeyX = solvedKeyX_1;
                                solvedKeyY = solvedKeyY - solvedKeyScale * mTotalSolvedKey->boundingBox().size.height;
                            }
                            else if (iMod == 28)
                            {
                                solvedKeyX = solvedKeyX_1 + solvedKeyScale * mTotalSolvedKey->boundingBox().size.width;
                                solvedKeyY = solvedKeyY - solvedKeyScale * mTotalSolvedKey->boundingBox().size.height;
                            }
                            else if (iMod == 31)
                            {
                                solvedKeyX = solvedKeyX_1 + 2 * solvedKeyScale * mTotalSolvedKey->boundingBox().size.width;
                                solvedKeyY = solvedKeyY - solvedKeyScale * mTotalSolvedKey->boundingBox().size.height;
                            }
                        }
                        
                        CCSprite* solvedKeySprite = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("menu-key"));
                        solvedKeySprite->retain();
                        solvedKeySprite->setPosition(ccp(solvedKeyX, solvedKeyY));
                        solvedKeySprite->setScale(solvedKeyScale);
                        mSolvedKeys->push_back(solvedKeySprite);
                        containerNode->addChild(solvedKeySprite);
                        
                        FloatTweener* tweenerX = new FloatTweener(0, this, CMTransitions::LINEAR);
                        tweenerX->setTag(kLevelUnlockedTagX + i);
                        tweenerX->reset(solvedKeySprite->getPositionX(),
                                        mTotalSolvedKey->getPositionX(),
                                        kSolvedKeyTweenDuration,
                                        (0.25f * i + 2) * kSolvedKeyTweenDelay);
                        mTweenersX->push_back(tweenerX);
                        
                        FloatTweener* tweenerY = new FloatTweener(0, this, CMTransitions::LINEAR);
                        tweenerY->setTag(kLevelUnlockedTagY + i);
                        tweenerY->reset(solvedKeySprite->getPositionY(),
                                        mTotalSolvedKey->getPositionY(),
                                        kSolvedKeyTweenDuration,
                                        (0.25f * i + 2) * kSolvedKeyTweenDelay);
                        mTweenersY->push_back(tweenerY);
                        
                        FloatTweener* tweenerRotation = new FloatTweener(0, this, CMTransitions::LINEAR);
                        tweenerRotation->setTag(kLevelUnlockedTagRotation + i);
                        tweenerRotation->reset(solvedKeySprite->getRotation(),
                                               (*mLevelIcons)[0]->getRotation() - 360,
                                               kSolvedKeyTweenDuration,
                                               (0.25f * i + 2) * kSolvedKeyTweenDelay);
                        mTweenersRotation->push_back(tweenerRotation);
                        
                        FloatTweener* tweenerScale = new FloatTweener(0, this, CMTransitions::LINEAR);
                        tweenerScale->setTag(kLevelUnlockedTagScale + i);
                        tweenerScale->reset(solvedKeySprite->getScale(),
                                            mTotalSolvedKey->getScale(),
                                            kSolvedKeyTweenDuration,
                                            (0.25f * i + 2) * kSolvedKeyTweenDelay);
                        mTweenersScale->push_back(tweenerScale);
                        
                        solvedKeyX += solvedKeyScale * mTotalSolvedKey->boundingBox().size.width;
                    }
                }
                break;
            case MenuCustomDialog::LEVEL_COMPLETE:
                {
                    CCNode* containerNode = new CCNode();
                    containerNode->setPosition(ccp(0, -70));
                    this->addChild(containerNode, kZOrderBg + 1);
                    containerNode->autorelease();
                    
                    mLevelHeader = TextUtils::createFX(SceneController::localizeString(LevelMenu::kLevelNames[mLevelIndex]), 72, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
                    mLevelHeader->setColor(CMUtils::uint2color3B(LevelMenu::kLevelColors[mLevelIndex]));
                    containerNode->addChild(mLevelHeader);
                    
                    if (mLevelHeader->boundingBox().size.width > kLevelHeaderWidthMax)
                        containerNode->setScale(kLevelHeaderWidthMax / mLevelHeader->boundingBox().size.width);
                    
                    float starOffsetY = mLevelHeader->getPositionY() + LangFX::getCustomDialogStarYOffset();
                    for (int i = 0; i < 2; ++i)
                    {
                        CCSprite* star = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("star-full"));
                        if (i == 0)
                        {
                            star->setPosition(ccp(
                                                  mLevelHeader->getPositionX() - (mLevelHeader->boundingBox().size.width / 2 + 0.6f * star->boundingBox().size.width),
                                                  starOffsetY + 0.5f * star->boundingBox().size.height));
                        }
                        else
                        {
                            star->setPosition(ccp(
                                                  mLevelHeader->getPositionX() + mLevelHeader->boundingBox().size.width / 2 + 0.6f * star->boundingBox().size.width,
                                                  starOffsetY + 0.5f * star->boundingBox().size.height));
                        }
                        
                        if (containerNode->getScale() != 0)
                            star->setScale(1.0f / containerNode->getScale());

						star->setColor(mLevelHeader->getColor());
						containerNode->addChild(star);
						mStars.push_back(star);
                    }
                }
                break;
			case MenuCustomDialog::WIZARD:
				{
					if (mStarsContainer)
						return true;

                    {
                        mStarsContainer = new CCNode();
                        mStarsContainer->setPosition(ccp(0, 64));
                        addChild(mStarsContainer, kZOrderBg + 1);

                        float angle = 0;
                        for (int i = 0; i < 8; ++i, angle += kPI / 4.0f)
                        {
                            CCSprite* star = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("star-full"));
                            star->setPosition(ccp(
                                                  cosf(angle) * kStarEllipseRadiusX,
                                                  sinf(angle) * kStarEllipseRadiusY));
                            star->setColor(ccc3((GLubyte)mRGB[i % 3], (GLubyte)mRGB[(i + 1) % 3], (GLubyte)mRGB[(i + 2) % 3]));
                            mStarsContainer->addChild(star);
                            mStars.push_back(star);
                        }
                    }
                    
                    {
                        CCNode* containerNode = new CCNode();
                        addChild(containerNode, kZOrderContent + 1);
                        containerNode->autorelease();
                        
                        // Applause
                        {
                            DelayedEvent* ev = DelayedEvent::create(kDelayedEvPlayCheerSound, 1.0f, this);
                            mScene->addToJuggler(ev);
                        }
                        
                        // Confetti
                        {
                            mConfetti = ParticleProp::createWithCategory(getCategory(), "confetti.plist", this, false);
                            refreshConfettiSettings();
                            mConfetti->stopSystem();
                            containerNode->addChild(mConfetti);
                            
                            DelayedEvent* ev = DelayedEvent::create(kDelayedEvDropConfetti, 1.0f, this, (void*)mConfetti);
                            mScene->addToJuggler(ev);
                        }
                        
                        // Fireworks
                        {
                            mFireworks = new std::vector<ParticleProp*>();
                            for (int i = 0; i < 4; ++i)
                            {
                                ParticleProp* firework = ParticleProp::createWithCategory(getCategory(), "fireworks.plist", this, false);
                                firework->stopSystem();
                                containerNode->addChild(firework);
                                mScene->addToJuggler(firework);
                                mFireworks->push_back(firework);
                                
                                DelayedEvent* ev = DelayedEvent::create(kDelayedEvLaunchFireworks, 2.0f + i * 1.0f, this, (void*)firework);
                                mScene->addToJuggler(ev);
                            }
                        }
                    }
				}
				break;
            case MenuCustomDialog::CREDITS:
                {
                    if (mScrollCropper)
                        return true;
                    
                    mScrollBounds = CCRectMake(0, 0, mScene->getViewWidth(), mScene->getViewHeight());
                    
                    mScrollCropper = new CroppedProp(getCategory(), mScrollBounds);
                    this->addChild(mScrollCropper, kZOrderContent + 1);
                    
                    mScrollContainer = new CCNode();
                    mScrollCropper->addChild(mScrollContainer);
                    
                    resetTouchTrail();
                }
                break;
			default:
				retVal = false;
				CCAssert(false, "Bad MenuCustomDialog type.");
				break;
		}
	}

	return retVal;
}

// Total retro-fit-hack, but works fine.
void MenuCustomDialog::repositionLevelCompleteLabels(void)
{
    Localizer::LocaleType locale = mScene->getLocale();
    
    CCLabelBMFont* levelLabel = static_cast<CCLabelBMFont*>(getContentItem(kLevelCompleteLabelTagA));
    if (levelLabel)
    {
        float oldOffset = localeCache == Localizer::INVALID_LOCALE ? 0 : LangFX::getLevelCompleteLabelOffsets(localeCache).val_0;
        levelLabel->setPositionY(levelLabel->getPositionY() + LangFX::getLevelCompleteLabelOffsets(locale).val_0 - oldOffset);
    }
    
    
    CCLabelBMFont* completedLabel = static_cast<CCLabelBMFont*>(getContentItem(kLevelCompleteLabelTagB));
    if (completedLabel)
    {
        float oldOffset = localeCache == Localizer::INVALID_LOCALE ? 0 : LangFX::getLevelCompleteLabelOffsets(localeCache).val_1;
        completedLabel->setPositionY(completedLabel->getPositionY() + LangFX::getLevelCompleteLabelOffsets(locale).val_1 - oldOffset);
    }
    
    // Set afterward so that first call initializes.
    if (localeCache == Localizer::INVALID_LOCALE)
        localeCache = mScene->getLocale();
}

void MenuCustomDialog::refreshConfettiSettings(void)
{
    if (mConfetti)
    {
        mConfetti->setPosition(0, 0.55f * mScene->getInvViewScale().y * mScene->getViewportHeight());
        CCPoint posVar = mConfetti->getSystem()->getPosVar();
        posVar.x = mScene->getInvViewScale().x * mScene->getViewportWidth();
        mConfetti->getSystem()->setPosVar(posVar);
    }
}

void MenuCustomDialog::randomizeFireworkPosition(ParticleProp* firework)
{
    float viewWidth = mScene->getViewWidth(), viewHeight = mScene->getViewHeight();
    CCRect rndRegion = CCRectZero;
    if (rndToggle)
        rndRegion.setRect(-0.35f * viewWidth,
                          0,
                          0.15f * viewWidth,
                          0.35f * viewHeight);
    else
        rndRegion.setRect(0.2f * viewWidth,
                          0,
                          0.15f * viewWidth,
                          0.35f * viewHeight);
    rndToggle = !rndToggle;
    
    firework->setPosition(ccp(
                              CMUtils::nextRandom(rndRegion.getMinX(), rndRegion.getMaxX()),
                              CMUtils::nextRandom(rndRegion.getMinY(), rndRegion.getMaxY())));
}

void MenuCustomDialog::recalculateViewableRegion(void)
{
    if (mScrollCropper)
    {
        CCPoint origin = convertToWorldSpace(mScrollBounds.origin);
        CCPoint topRight = convertToWorldSpace(ccp(mScrollBounds.getMaxX(), mScrollBounds.getMaxY()));
        CCRect viewableRegion = CCRectMake(origin.x, origin.y, topRight.x - origin.x, topRight.y - origin.y);
        mScrollCropper->setViewableRegion(viewableRegion);
    }
}

void MenuCustomDialog::resolutionDidChange(void)
{
    MenuDialog::resolutionDidChange();
    
    recalculateViewableRegion();
    
    switch (mType)
    {
        case MenuCustomDialog::WIZARD:
            refreshConfettiSettings();
            break;
        default:
            break;
    }
}

void MenuCustomDialog::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    MenuDialog::localeDidChange(fontKey, FXFontKey);
    
    if (mLevelIndex < 0 || mLevelIndex >= LevelMenu::kNumLevels)
        return;
    
    switch (mType)
	{
        case MenuCustomDialog::LEVEL_UNLOCKED:
            {
                if (mLevelIcons)
                {
                    for (std::vector<LevelIcon*>::iterator it = mLevelIcons->begin(); it != mLevelIcons->end(); ++it)
                        (*it)->localeDidChange(fontKey, FXFontKey);
                }
            }
            break;
        case MenuCustomDialog::WIZARD_UNLOCKED:
            {
                if (mLevelIcons)
                {
                    for (std::vector<LevelIcon*>::iterator it = mLevelIcons->begin(); it != mLevelIcons->end(); ++it)
                        (*it)->localeDidChange(fontKey, FXFontKey);
                }
                
                TextUtils::switchFntFile(fontKey, mTotalSolvedLabel, false);
                
                if (mTotalSolvedLabel && mTotalSolvedKey)
                {
                    mTotalSolvedKey->setPosition(ccp(
                                                     mTotalSolvedKey->getPositionX(),
                                                     mTotalSolvedLabel->getPositionY() + LangFX::getCustomDialogKeyYOffset()));
                }
            }
            break;
        case MenuCustomDialog::LEVEL_COMPLETE:
            {
                if (mLevelHeader)
                {
                    CCNode* containerNode = mLevelHeader->getParent();
                    if (containerNode)
                    {
                        containerNode->setScale(1.0f);
                        TextUtils::switchFntFile(FXFontKey, mLevelHeader);
                        
                        if (mLevelHeader->boundingBox().size.width > kLevelHeaderWidthMax)
                            containerNode->setScale(kLevelHeaderWidthMax / mLevelHeader->boundingBox().size.width);
                        
                        float starOffsetY = mLevelHeader->getPositionY() + LangFX::getCustomDialogStarYOffset();
                        for (int i = 0; i < mStars.size(); ++i)
                        {
                            CCSprite* star = mStars[i];
                            star->setScale(1.0f);
                            if (i == 0)
                            {
                                star->setPosition(ccp(
                                                      mLevelHeader->getPositionX() - (mLevelHeader->boundingBox().size.width / 2 + 0.6f * star->boundingBox().size.width),
                                                      starOffsetY + 0.5f * star->boundingBox().size.height));
                            }
                            else
                            {
                                star->setPosition(ccp(
                                                      mLevelHeader->getPositionX() + mLevelHeader->boundingBox().size.width / 2 + 0.6f * star->boundingBox().size.width,
                                                      starOffsetY + 0.5f * star->boundingBox().size.height));
                            }
                            
                            if (containerNode->getScale() != 0)
                                star->setScale(1.0f / containerNode->getScale());
                        }
                    }
                }
                
                repositionLevelCompleteLabels();
                localeCache = mScene->getLocale();
            }
            break;
        case MenuCustomDialog::WIZARD:
            break;
        case MenuCustomDialog::CREDITS:
            {
                if (mScrollContainer)
                {
                    CCArray* children = mScrollContainer->getChildren();
                    if (children)
                    {
                        CCObject* obj;
                        CCARRAY_FOREACH(children, obj)
                        {
                            CCLabelBMFont* label = dynamic_cast<CCLabelBMFont*>(obj);
                            if (label)
                            {
                                if (label->getTag() != ILocalizable::kNonLocalizableTag)
                                    TextUtils::switchFntFile(fontKey, label);
                            }
                            else
                            {
                                ILocalizable* localizable = dynamic_cast<ILocalizable*>(obj);
                                if (localizable)
                                    localizable->localeDidChange(fontKey, FXFontKey);
                            }
                        }
                    }
                }
            }
            break;
    }
}

void MenuCustomDialog::show(AnimState anim)
{
    MenuDialog::show(anim);
    
	switch (mType)
	{
        case MenuCustomDialog::CREDITS:
            {
                mScrollRate = kDefaultScrollRate;
                mScrollContainer->setPositionY(mScrollBounds.getMinY() + 20);
                
                recalculateViewableRegion();
            }
            break;
        default:
            break;
    }
}

void MenuCustomDialog::addScrollingItem(CCNode* item)
{
    if (item && mScrollContainer)
    {
        mScrollContainer->addChild(item);
        mScrollContentBounds = CMUtils::unionRect(mScrollContentBounds, item->boundingBox());
    }
}

void MenuCustomDialog::removeScrollingItem(CCNode* item)
{
    if (item && mScrollContainer)
    {
        mScrollContainer->removeChild(item);
        mScrollContentBounds = CCRectZero;
        CCArray* children = mScrollContainer->getChildren();
        
        if (children)
        {
            CCObject* obj;
            CCARRAY_FOREACH(children, obj)
            {
                CCNode* child = static_cast<CCNode*>(obj);
                if (child)
                    mScrollContentBounds = CMUtils::unionRect(mScrollContentBounds,  child->boundingBox());
            }
        }
    }
}

bool MenuCustomDialog::isTouchEnabled(void)
{
    return isVisible();
}

void MenuCustomDialog::applyTouchTrailMomentum(void)
{
    int count = 0;
    float momentum = 0;
    for (int i = 0; i < kTouchTrailLen; ++i)
    {
        if (mTouchTrail[i] == -1)
            continue;
        momentum += 0.5f * mTouchTrail[i] * ((kTouchTrailLen - count) / (float)kTouchTrailLen);
        ++count;
    }
    
    mScrollRate = count > 0 ? momentum / count : 0;
    //CCLog("Scroll rate: %f", mScrollRate);
    if (fabsf(mScrollRate) < 0.1f)
    {
        mScrollRate = 0.1f;
    }
    else if (fabsf(mScrollRate) > kMaxScrollRate)
    {
        mScrollRate = (mScrollRate > 0 ? 1 : -1) * kMaxScrollRate;
    }
}

void MenuCustomDialog::resetTouchTrail(void)
{
    for (int i = 0; i < kTouchTrailLen; ++i)
        mTouchTrail[i] = -1;
}

void MenuCustomDialog::setNextTouchTrail(float value)
{
    if (mTrailIndex >= kTouchTrailLen)
        mTrailIndex = 0;
    mTouchTrail[mTrailIndex++] = value;
}

void MenuCustomDialog::onTouch(int evType, void* evData)
{
    if (!isTouchEnabled())
		return;
    
	CMTouches::TouchNotice* touchNotice = (CMTouches::TouchNotice*)evData;
	if (touchNotice == NULL)
		return;
    
    CCPoint touchPos = convertToNodeSpace(touchNotice->pos);
    CCPoint prevTouchPos = convertToNodeSpace(touchNotice->prevPos);
    CCPoint deltaPos = ccp(touchPos.x - prevTouchPos.x, touchPos.y - prevTouchPos.y);
    
    if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN() && !mScrollBounds.containsPoint(touchPos))
        return;
    if (!mIsTouched && evType != TouchPad::EV_TYPE_TOUCH_BEGAN())
        return;
    
	touchNotice->retainFocus(this);
    
	if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN())
	{
		mIsTouched = true;
        mTouchDelta = 0;
	}
	else if (evType == TouchPad::EV_TYPE_TOUCH_ENDED() || evType == TouchPad::EV_TYPE_TOUCH_CANCELLED())
	{
		mIsTouched = false;
        applyTouchTrailMomentum();
        resetTouchTrail();
	}
	else if (evType == TouchPad::EV_TYPE_TOUCH_MOVED())
	{
        float touchDeltaY = deltaPos.y;
        if ((mTouchDelta > 0 && touchDeltaY < 0) || (mTouchDelta < 0 && touchDeltaY > 0))
            resetTouchTrail();
		mTouchDelta = touchDeltaY;
        setNextTouchTrail(mTouchDelta);
        
        mScrollContainer->setPositionY(mScrollContainer->getPositionY() + mTouchDelta);
        float scrollHgtOffset = mScrollContentBounds.origin.y + mScrollContentBounds.size.height;

        if (mTouchDelta > 0)
        {
        	if (mScrollContainer->getPositionY() + scrollHgtOffset > mScrollBounds.getMaxY() + mScrollContentBounds.size.height)
        		mScrollContainer->setPositionY(mScrollBounds.getMinY() - scrollHgtOffset);
        }
        else if (mTouchDelta < 0)
        {
        	if (mScrollContainer->getPositionY() + scrollHgtOffset < mScrollBounds.getMinY())
        		mScrollContainer->setPositionY(mScrollBounds.getMaxY() + mScrollContentBounds.size.height);
        }
	}
}

void MenuCustomDialog::onEvent(int evType, void* evData)
{
    MenuDialog::onEvent(evType, evData);
    
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
            int tag = tweener->getTag();
            
            if (tag >= kLevelUnlockedTagLockDrop)
            {
                if (mLevelUnlocked)
                    mLevelUnlocked->setPositionY(tweener->getTweenedValue());
            }
            else if (tag >= kLevelUnlockedTagArrowX)
            {
                if (mLevelArrow)
                    mLevelArrow->setPositionX(tweener->getTweenedValue());
            }
            else if (tag >= kLevelUnlockedTagScale)
            {
                int index = tag - kLevelUnlockedTagScale;
                if (mTweenersScale && mSolvedKeys && index >= 0 && index < mTweenersScale->size() && index < mSolvedKeys->size())
                {
                    (*mSolvedKeys)[index]->setScale((*mTweenersScale)[index]->getTweenedValue());
                }
            }
            else if (tag >= kLevelUnlockedTagRotation)
            {
                int index = tag - kLevelUnlockedTagRotation;
                if (mTweenersRotation && mSolvedKeys && index >= 0 && index < mTweenersRotation->size() && index < mSolvedKeys->size())
                {
                    (*mSolvedKeys)[index]->setRotation((*mTweenersRotation)[index]->getTweenedValue());
                }
            }
            else if (tag >= kLevelUnlockedTagY)
            {
                int index = tag - kLevelUnlockedTagY;
                if (mTweenersY && mSolvedKeys && index >= 0 && index < mTweenersY->size() && index < mSolvedKeys->size())
                {
                    (*mSolvedKeys)[index]->setPositionY((*mTweenersY)[index]->getTweenedValue());
                }
            }
            else if (tag >= kLevelUnlockedTagX)
            {
                int index = tag - kLevelUnlockedTagX;
                if (mTweenersX && mSolvedKeys && index >= 0 && index < mTweenersX->size() && index < mSolvedKeys->size())
                {
                    (*mSolvedKeys)[index]->setPositionX((*mTweenersX)[index]->getTweenedValue());
                }
            }
        }
    }
    else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
    {
        if (getType() == MenuCustomDialog::LEVEL_UNLOCKED)
        {
            FloatTweener* tweener = static_cast<FloatTweener*>(evData);
            if (tweener)
            {
                int tag = tweener->getTag();
                
                if (tag == kLevelUnlockedTagLockDrop)
                {
                    if (mLevelUnlocked)
                        mLevelUnlocked->setVisible(false);
                }
                else if (tag == kLevelUnlockedTagArrowX)
                {
                    dropLockOverTime(0.75f, 0.5f);
                }
                else if (tag >= kLevelUnlockedTagScale && tag < kLevelUnlockedTagArrowAlpha)
                {
                    if (mSolvedKeys && mSolvedCount < mSolvedKeys->size() && (*mSolvedKeys)[mSolvedCount])
                    {
                        (*mSolvedKeys)[mSolvedCount]->setVisible(false);
                        
                        if (mLevelIcons && mLevelIcons->size() > 0 && (*mLevelIcons)[0])
                        {
                            if (mTweenerGlow == NULL)
                            {
                                mTweenerGlow = new ByteTweener(0, this, CMTransitions::LINEAR);
                                mTweenerGlow->setTag(kLevelUnlockedTagGlow);
                            }
                            
                            mSolvedGlowDir = 1;
                            mTweenerGlow->reset((*mLevelIcons)[0]->getSolvedGlowOpacity(), 255, 0.5f * kSolvedKeyTweenDelay);
                        }
                        
                        if (++mSolvedCount == kSolvedKeyCount)
                            showArrowOverTime(0.4f, 0.75f);
                        
                        mScene->playSound(mSolvedCount < kSolvedKeyCount ? "solved-short" : "solved");
                        
                        if (mLevelIcons && mLevelIcons->size() > 0 && (*mLevelIcons)[0])
                            (*mLevelIcons)[0]->setPuzzlesSolved(mSolvedCount, PuzzlePage::kNumPuzzlesPerPage);
                    }
                }
            }
        }
        else if (getType() == MenuCustomDialog::WIZARD_UNLOCKED)
        {
            FloatTweener* tweener = static_cast<FloatTweener*>(evData);
            if (tweener)
            {
                int tag = tweener->getTag();
            
                if (tag >= kLevelUnlockedTagScale && tag < kLevelUnlockedTagArrowAlpha)
                {
                    if (mTotalSolvedLabel && mSolvedKeys && mTotalSolvedCount < mSolvedKeys->size() && (*mSolvedKeys)[mTotalSolvedCount])
                    {
                        (*mSolvedKeys)[mTotalSolvedCount]->setVisible(false);
                        
                        ++mTotalSolvedCount;
                        mTotalSolvedLabel->setString(CMUtils::strConcatVal(CMUtils::strConcatVal("", mTotalSolvedCount).append("/").c_str(), kWizardKeyCount).c_str());
                        
                        if (mTotalSolvedCount == kWizardKeyCount)
                        {
                            mTotalSolvedLabel->setColor(CMUtils::uint2color3B(0x22ff3e));
                            mScene->playSound("solved");
                            mDropLockFrameCountdown = 60;
                        }
                        else
                        {
                            mScene->playSound("solved-short");
                        }
                    }
                }
                else if (tag == kLevelUnlockedTagLockDrop)
                {
                    if (mLevelUnlocked)
                        mLevelUnlocked->setVisible(false);
                }
            }
        }
    }
    else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
    {
        ByteTweener* tweener = static_cast<ByteTweener*>(evData);
        if (tweener)
        {
            int tag = tweener->getTag();
            GLubyte tweenedValue = tweener->getTweenedValue();
            
            if (tag >= kLevelUnlockedTagGlow)
            {
                if (mLevelIcons && mLevelIcons->size() > 0 && (*mLevelIcons)[0])
                    (*mLevelIcons)[0]->setSolvedGlowOpacity(tweenedValue);
            }
            else if (tag == kLevelUnlockedTagArrowAlpha)
            {
                if (mLevelArrow)
                    mLevelArrow->setOpacity(tweenedValue);
            }
        }
    }
    else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
    {
        ByteTweener* tweener = static_cast<ByteTweener*>(evData);
        if (tweener)
        {
            int tag = tweener->getTag();
            if (tag == kLevelUnlockedTagGlow)
            {
                if (mSolvedGlowDir == 1)
                {
                    mSolvedGlowDir = -1;
                    tweener->reverse();
                }
            }
        }
    }
    else if (evType == ParticleProp::EV_TYPE_DID_COMPLETE())
    {
        if (evData)
        {
            DelayedEvent* ev = DelayedEvent::create(kDelayedEvLaunchFireworks, 1.0f + CMUtils::nextRandom(0, 10) * 0.05f, this, evData);
            mScene->addToJuggler(ev);
        }
    }
    else if (evType == DelayedEvent::EV_TYPE_DID_EXPIRE())
    {
        if (evData)
        {
            DelayedEvent* delayedEvent = (DelayedEvent*)evData;
            if (delayedEvent->getEventTag() == kDelayedEvLaunchFireworks)
            {
                ParticleProp* firework = (ParticleProp*)delayedEvent->getUserInfo();
                if (firework)
                {
                    randomizeFireworkPosition(firework);
                    firework->resetSystem(1.0f);
                    unsigned int soundId = mScene->playSound("fireworks");
                    mFireworkSoundIds.insert(mFireworkSoundIds.begin(), soundId);
                    if (mFireworkSoundIds.size() > 4)
                        mFireworkSoundIds.pop_back();
                }
            }
            else if (delayedEvent->getEventTag() == kDelayedEvDropConfetti)
            {
                if (mConfetti)
                    mConfetti->resetSystem();
            }
            else if (delayedEvent->getEventTag() == kDelayedEvPlayCheerSound)
            {
                mScene->playSound("crowd-cheer");
            }
        }
    }
}

void MenuCustomDialog::showArrowOverTime(float duration, float delay)
{
    if (mLevelArrow == NULL)
        return;
    
    if (mTweenerArrowX == NULL)
    {
        mTweenerArrowX = new FloatTweener(0, this, CMTransitions::LINEAR);
        mTweenerArrowX->setTag(kLevelUnlockedTagArrowX);
    }
    
    if (mTweenerArrowAlpha == NULL)
    {
        mTweenerArrowAlpha = new ByteTweener(0, this, CMTransitions::LINEAR);
        mTweenerArrowAlpha->setTag(kLevelUnlockedTagArrowAlpha);
    }
    
    mLevelArrow->setPositionX(-kLevelUnlockedWidthMax / 2);
    mLevelArrow->setVisible(true);
    mTweenerArrowX->reset(mLevelArrow->getPositionX(), 0, duration, delay);
    mTweenerArrowAlpha->reset(mLevelArrow->getOpacity(), 255, duration / 3, delay);
}

void MenuCustomDialog::dropLockOverTime(float duration, float delay)
{
    if (mLevelLock == NULL || mLevelUnlocked == NULL)
        return;
    
    if (mTweenerLockY == NULL)
    {
        mTweenerLockY = new FloatTweener(0, this, CMTransitions::EASE_IN);
        mTweenerLockY->setTag(kLevelUnlockedTagLockDrop);
    }
    
    mTweenerLockY->reset(mLevelUnlocked->getPositionY(), mLevelUnlocked->getPositionY() - 0.7f * mScene->getViewHeight(), duration, delay);
    mLevelLock->setVisible(false);
    mLevelUnlocked->setVisible(true);
    mScene->playSound("unlocked");
}

void MenuCustomDialog::update(int controllerState)
{
    MenuDialog::update(controllerState);

    InputManager* im = InputManager::IM();
    Coord heldVec = im->getHeldVector();
    
    if (heldVec.y != 0)
    {
        mScrollRateDelta = -0.2f * MAX(kDefaultScrollRate, fabsf(mScrollRate)) * heldVec.y;
        mScrollRate += mScrollRateDelta;
        if (fabsf(mScrollRate) > kMaxScrollRate)
        {
            mScrollRate = (mScrollRate > 0 ? 1 : -1) * kMaxScrollRate;
        }
    }
    else
        mScrollRateDelta = 0;
}

void MenuCustomDialog::advanceTime(float dt)
{
	MenuDialog::advanceTime(dt);

	switch (mType)
	{
        case MenuCustomDialog::LEVEL_UNLOCKED:
            {
                if (mTweenersX)
                {
                    for (std::vector<FloatTweener*>::iterator it = mTweenersX->begin(); it != mTweenersX->end(); ++it)
                        (*it)->advanceTime(dt);
                }
                
                if (mTweenersY)
                {
                    for (std::vector<FloatTweener*>::iterator it = mTweenersY->begin(); it != mTweenersY->end(); ++it)
                        (*it)->advanceTime(dt);
                }
                
                if (mTweenersRotation)
                {
                    for (std::vector<FloatTweener*>::iterator it = mTweenersRotation->begin(); it != mTweenersRotation->end(); ++it)
                        (*it)->advanceTime(dt);
                }
                
                if (mTweenersScale)
                {
                    for (std::vector<FloatTweener*>::iterator it = mTweenersScale->begin(); it != mTweenersScale->end(); ++it)
                        (*it)->advanceTime(dt);
                }
                
                if (mTweenerArrowAlpha)
                    mTweenerArrowAlpha->advanceTime(dt);
                
                if (mTweenerArrowX)
                    mTweenerArrowX->advanceTime(dt);
                
                if (mTweenerLockY)
                    mTweenerLockY->advanceTime(dt);
                
                if (mTweenerGlow)
                    mTweenerGlow->advanceTime(dt);
            }
            break;
        case MenuCustomDialog::WIZARD_UNLOCKED:
            {
                if (mTweenersX)
                {
                    for (std::vector<FloatTweener*>::iterator it = mTweenersX->begin(); it != mTweenersX->end(); ++it)
                        (*it)->advanceTime(dt);
                }
                
                if (mTweenersY)
                {
                    for (std::vector<FloatTweener*>::iterator it = mTweenersY->begin(); it != mTweenersY->end(); ++it)
                        (*it)->advanceTime(dt);
                }
                
                if (mTweenersRotation)
                {
                    for (std::vector<FloatTweener*>::iterator it = mTweenersRotation->begin(); it != mTweenersRotation->end(); ++it)
                        (*it)->advanceTime(dt);
                }
                
                if (mTweenersScale)
                {
                    for (std::vector<FloatTweener*>::iterator it = mTweenersScale->begin(); it != mTweenersScale->end(); ++it)
                        (*it)->advanceTime(dt);
                }
                
                if (mTweenerLockY)
                    mTweenerLockY->advanceTime(dt);
                
                if (mDropLockFrameCountdown > 0)
                {
                    --mDropLockFrameCountdown;
                    if (mDropLockFrameCountdown == 0)
                        dropLockOverTime(0.75f, 0.5f);
                }
            }
            break;
        case MenuCustomDialog::LEVEL_COMPLETE:
            {
                int numStars = (int)mStars.size();
                for (int i = 0; i < numStars; ++i)
                    mStars[i]->setRotation(mStars[i]->getRotation() - ((i & 1) == 1 ? -30 : 30) * dt);
            }
            break;
		case MenuCustomDialog::WIZARD:
			{
				for (int i = 0; i < 3; ++i)
                {
                    mRGB[i] = mRGB[i] + mRGBIncrements[i];
                    if (mRGB[i] < 0 || mRGB[i] >= 256)
                    {
                        mRGBIncrements[i] *= -1;
                        mRGB[i] = MAX(0, MIN(mRGB[i], 255));
                    }
                }

                mStarAngle -= dt / 4.0f;
                int numStars = (int)mStars.size();
                float angle = mStarAngle;
                for (int i = 0; i < numStars; ++i, angle += kPI / (numStars / 2.0f))
                {
                    CCSprite* star = mStars[i];
                    star->setPosition(ccp(
						cosf(angle) * kStarEllipseRadiusX,
						sinf(angle) * kStarEllipseRadiusY));
					star->setRotation(star->getRotation() - 30 * dt);
					star->setColor(ccc3((GLubyte)mRGB[i % 3], (GLubyte)mRGB[(i + 1) % 3], (GLubyte)mRGB[(i + 2) % 3]));
                }
			}
			break;
        case MenuCustomDialog::CREDITS:
            {
                if (mIsTouched)
                {
                    if (mTrailIndex >= kTouchTrailLen)
                        mTrailIndex = 0;
                    mTouchTrail[mTrailIndex++] = mTouchDelta;
                }
                else
                {
                    mScrollContainer->setPositionY(mScrollContainer->getPositionY() + mScrollRate);
                    float scrollHgtOffset = mScrollContentBounds.origin.y + mScrollContentBounds.size.height;
                    if (mScrollRate > 0)
                    {
                        if (mScrollContainer->getPositionY() + scrollHgtOffset > mScrollBounds.getMaxY() + mScrollContentBounds.size.height)
                            mScrollContainer->setPositionY(mScrollBounds.getMinY() - scrollHgtOffset);
                    }
                    else if (mScrollRate < 0)
                    {
                        if (mScrollContainer->getPositionY() + scrollHgtOffset < mScrollBounds.getMinY())
                            mScrollContainer->setPositionY(mScrollBounds.getMaxY() + mScrollContentBounds.size.height);
                    }
                    
                    float prevScrollRate = mScrollRate;
                    if (mScrollRateDelta == 0 && mScrollRate > 0 && mScrollRate < kDefaultScrollRate)
                        mScrollRate *= 1.11f;
                    else
                        mScrollRate *= kScrollRateDecelFactor;
                    
                    if (mScrollRateDelta == 0 && mScrollRate < kDefaultScrollRate && prevScrollRate >= kDefaultScrollRate) // Clamp to default scroll rate
                        mScrollRate = kDefaultScrollRate;
                    else if (mScrollRateDelta == 0 && mScrollRate > -0.05f && mScrollRate < 0) // Reverse direction
                        mScrollRate = 0.05f * kDefaultScrollRate;
                }
            }
            break;
	}
}
