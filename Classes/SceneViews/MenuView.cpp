
#include "MenuView.h"
#include <SceneControllers/MenuController.h>
#include <SceneControllers/GameController.h>
#include <Prop/UIViews/MenuSubview.h>
#include <Puzzle/View/Menu/LevelMenu.h>
#include <Utils/ByteTweener.h>
#include <Utils/Transitions.h>
#include <Utils/Globals.h>
#include <Utils/Utils.h>
#include <algorithm>
USING_NS_CC;

typedef std::map<std::string, MenuSubview*> SVMap;
typedef std::vector<MenuSubview*> SVStack;

static const float kMinSplashTime = 2.0f;

MenuView::MenuView(int category, MenuController* controller)
	:
	Prop(category),
    mSplashTimer(-1),
    mBgQuad(NULL),
    mSplashNode(NULL),
    mSplashProp(NULL),
	mLevelMenu(NULL)
{
	mController = controller;
}

MenuView::~MenuView(void)
{
	mSubviewStack.clear();
    mScene->deregisterResDependent(this);
    mScene->unsubscribeToInputUpdates(this);
    mScene->removeTweensWithTarget(this);
	unpopulateLevelMenuView();
	
	for (SVMap::iterator it = mSubviews.begin(); it != mSubviews.end(); ++it)
		CC_SAFE_RELEASE_NULL(it->second);
	mSubviews.clear();
    
    if (mSplashProp)
    {
        mSplashProp->removeFromParent();
        mSplashProp = NULL;
    }

    mBgQuad = NULL;
    mSplashNode = NULL;
	mController = NULL;
}

bool MenuView::init(void)
{
	bool bRet = Prop::init();
	do
	{
		if (!bRet) break;

		MenuSubview* levelMenuSubview = new MenuSubview();
		bRet = levelMenuSubview && levelMenuSubview->init();
		if (!bRet) break;
		mSubviews["LevelMenu"] = levelMenuSubview;

        mSplashProp = Prop::createWithCategory(CMGlobals::SPLASH);
        
        CCSize winSize = CCDirector::sharedDirector()->getWinSize();
        mBgQuad = GameController::GC()->createBgQuad();
        mSplashProp->addChild(mBgQuad);
        
        float maximizingScale = GameController::getMaximizingScale();
        mSplashNode = GameController::GC()->createSplashNode();
        mSplashNode->setScale(maximizingScale);
        mSplashNode->setPosition(ccp(winSize.width / 2, winSize.height / 2));
        mSplashProp->addChild(mSplashNode);
        mScene->getCCScene()->addChild(mSplashProp);
        
        mScene->registerResDependent(this);
        mScene->subscribeToInputUpdates(this);
	} while (false);

	return bRet;
}

void MenuView::attachEventListeners(void)
{

}

void MenuView::detachEventListeners(void)
{

}

void MenuView::resolutionDidChange(void)
{
    if (mSplashNode && mBgQuad)
    {
        CCSize winSize = CCDirector::sharedDirector()->getWinSize();
        mBgQuad->setTextureRect(CCRectMake(0, 0, winSize.width, winSize.height));
        mBgQuad->setPosition(ccp(mBgQuad->boundingBox().size.width / 2, mBgQuad->boundingBox().size.height / 2));
        
        float maximizingScale = MIN(winSize.width / 1024.0f, winSize.height / 768.0f);
        mSplashNode->setScale(maximizingScale);
        mSplashNode->setPosition(ccp(winSize.width / 2, winSize.height / 2));
    }
}

MenuSubview* MenuView::getCurrentSubview(void) const
{
	if (mSubviewStack.size() > 0)
		return mSubviewStack[mSubviewStack.size()-1];
	else
		return NULL;
}

MenuSubview* MenuView::subviewForKey(const char* key)
{
	MenuSubview* subview = NULL;
	if (key)
	{
		std::string stdKey(key);
		if (!stdKey.empty())
		{
			SVMap::iterator it = mSubviews.find(stdKey);
			subview = it != mSubviews.end() ? (*it).second : NULL;
		}
	}

	return subview;
}

MenuSubview* MenuView::subviewForTag(int tag)
{
	MenuSubview* subview = NULL;

	for (SVMap::iterator it = mSubviews.begin(); it != mSubviews.end(); ++it)
	{
		if (it->second->getTag() == tag)
		{
			subview = it->second;
			break;
		}
	}

	return subview;
}

std::string MenuView::keyForTag(int tag)
{
	std::string key;
	for (SVMap::iterator it = mSubviews.begin(); it != mSubviews.end(); ++it)
	{
		if (it->second->getTag() == tag)
		{
			key = it->first;
			break;
		}
	}

	return key;
}

void MenuView::pushSubviewForKey(const char* key)
{
	pushSubview(subviewForKey(key));
}

void MenuView::pushSubview(MenuSubview* subview)
{
	if (subview)
	{
        SVStack::iterator findIt = std::find(mSubviewStack.begin(), mSubviewStack.end(), subview);
        if (findIt == mSubviewStack.end())
        {
            subview->setVisible(true);
            mSubviewStack.push_back(subview);
            addChild(subview);
        }
	}
}

void MenuView::popSubview(void)
{
	if (mSubviewStack.size() > 0)
	{
		MenuSubview* subview = mSubviewStack.back();
		subview->setVisible(false);
		subview->removeFromParent();
		//mScene->removeTweensWithTarget((IEventListener*)subview); // We're the target, not the subview.
		mSubviewStack.pop_back();
	}
}

void MenuView::popAllSubviews(void)
{
	while (mSubviewStack.size() > 1)
		popSubview();
}

void MenuView::destroySubviewForKey(const char* key)
{
	MenuSubview* subview = subviewForKey(key);
	if (subview)
	{
#if DEBUG
        SVStack::iterator findIt = std::find(mSubviewStack.begin(), mSubviewStack.end(), subview);
		CCAssert(findIt == mSubviewStack.end(), "MenuView: Attempt to destroy a subview while it is still on the stack.");
#endif
		mSubviews.erase(std::string(key));
		CC_SAFE_RELEASE_NULL(subview);
	}
}

void MenuView::destroySubview(MenuSubview* subview)
{
	if (subview)
		destroySubviewForKey(keyForTag(subview->getTag()).c_str());
}

void MenuView::populateTitleSubview(void)
{
	MenuSubview* subview = subviewForKey("Title");
	if (subview == NULL)
	{
		subview = new MenuSubview();
		mSubviews["Title"] = subview;

		/*CCSprite* quad = CMUtils::createColoredQuad(CCSizeMake(mScene->getFullscreenWidth(), mScene->getFullscreenHeight()));
		quad->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
		quad->setColor(ccc3(0,0,0));
		subview->addChild(quad);

		CCSprite* sprite = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("game-title"));
		sprite->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
		subview->addChild(sprite);*/
	}
}

void MenuView::fadeSubviewOverTime(const char* key, float duration, float delay, bool destroy)
{
	MenuSubview* subview = subviewForKey(key);
	if (subview)
	{
		subview->setShouldDestroy(destroy);

		ByteTweener* tweener = new ByteTweener(0, this, CMTransitions::LINEAR);
		tweener->setTag(subview->getTag());
		tweener->reset(subview->getOpacity(), 0, duration, delay);
		mScene->addToJuggler(tweener);
	}
}

void MenuView::showLevelMenuOverTime(float duration)
{
	if (mLevelMenu)
		mLevelMenu->showOverTime(duration);
}

void MenuView::hideLevelMenuOverTime(float duration)
{
	if (mLevelMenu)
		mLevelMenu->hideOverTime(duration);
}

void MenuView::hideLevelMenuInstantaneously(void)
{
    if (mLevelMenu)
        mLevelMenu->hideInstantaneously();
}

void MenuView::enableRateTheGameIcon(bool enable)
{
    if (mLevelMenu)
        mLevelMenu->enableRateTheGameIcon(enable);
}

void MenuView::animateRateTheGameIcon(bool enable)
{
    if (mLevelMenu)
        mLevelMenu->animateRateTheGameIcon(enable);
}

void MenuView::populateLevelMenuView(CCArray* levels)
{
	if (mLevelMenu)
		return;

	MenuSubview* subview = subviewForKey("LevelMenu");
	mLevelMenu = new LevelMenu(getCategory(), levels);
	mLevelMenu->init();
	//mLevelMenu->setPosition(ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
	mLevelMenu->addEventListener(LevelMenu::EV_TYPE_DID_TRANSITION_IN(), this);
	mLevelMenu->addEventListener(LevelMenu::EV_TYPE_DID_TRANSITION_OUT(), this);
	mLevelMenu->addEventListener(LevelMenu::EV_TYPE_PUZZLE_SELECTED(), this);
	mLevelMenu->addEventListener(LevelMenu::EV_TYPE_DEVICE_BACK_CLICKED(), this);
    mLevelMenu->addEventListener(LevelMenu::EV_TYPE_RATE_THE_GAME(), this);
	subview->addChild(mLevelMenu);
    mScene->subscribeToInputUpdates(mLevelMenu);
}

void MenuView::unpopulateLevelMenuView(void)
{
	if (mLevelMenu)
	{
		MenuSubview* subview = subviewForKey("LevelMenu");
		if (subview)
			subview->removeChild(mLevelMenu);
		mLevelMenu->removeEventListener(LevelMenu::EV_TYPE_DID_TRANSITION_IN(), this);
		mLevelMenu->removeEventListener(LevelMenu::EV_TYPE_DID_TRANSITION_OUT(), this);
		mLevelMenu->removeEventListener(LevelMenu::EV_TYPE_PUZZLE_SELECTED(), this);
		mLevelMenu->removeEventListener(LevelMenu::EV_TYPE_DEVICE_BACK_CLICKED(), this);
        mLevelMenu->removeEventListener(LevelMenu::EV_TYPE_RATE_THE_GAME(), this);
        mScene->unsubscribeToInputUpdates(mLevelMenu);
		mLevelMenu->removeFromParent();
		CC_SAFE_RELEASE_NULL(mLevelMenu);
	}
}

void MenuView::onEvent(int evType, void* evData)
{
	if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
		{
			MenuSubview* subview = subviewForTag(tweener->getTag());
			if (subview)
				subview->setOpacityChildren(tweener->getTweenedValue());
		}
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
		{
			MenuSubview* subview = subviewForTag(tweener->getTag());
			if (subview && subview->shouldDestroy())
			{
				if (subview == getCurrentSubview())
					popSubview();
				destroySubview(subview);
			}
			tweener->setListener(NULL);
            mScene->removeFromJuggler(tweener);
			tweener->autorelease();
		}
	}
	else if (evType == LevelMenu::EV_TYPE_PUZZLE_SELECTED())
	{
		mController->puzzleWasSelectedAtMenu(mLevelMenu->getSelectedPuzzleID());
	}
	else if (evType == LevelMenu::EV_TYPE_DID_TRANSITION_IN())
	{
		mController->onEvent(evType, evData);
	}
	else if (evType == LevelMenu::EV_TYPE_DID_TRANSITION_OUT())
	{
		mController->onEvent(evType, evData);
	}
	else if (evType == LevelMenu::EV_TYPE_DEVICE_BACK_CLICKED())
	{
		mController->onEvent(evType, evData);
	}
    else if (evType == LevelMenu::EV_TYPE_RATE_THE_GAME())
    {
        mController->onEvent(evType, evData);
    }
    else if (evType == EV_TYPE_SPLASH_DID_HIDE())
    {
        mController->onEvent(EV_TYPE_SPLASH_DID_HIDE(), evData);
    }
}

void MenuView::update(int controllerState)
{
#ifndef __ANDROID__
    if (mController->getState() != MenuController::MENU_IN)
        return;
    
    if (ControlsManager::CM()->didKeyDepress(kVK_Escape))
        mScene->showEscDialog();
#endif
}

void MenuView::advanceTime(float dt)
{
	if (mController)
	{
		if (mController->getState() == MenuController::MENU_IN)
		{
			MenuSubview* subview = getCurrentSubview();
			if (subview)
				subview->advanceTime(dt);
		}

		if (mLevelMenu && mController->getState() != MenuController::MENU_OUT)
			mLevelMenu->advanceTime(dt);
        
        if (mSplashProp)
        {
            if (mSplashTimer < 0)
            {
                // Initialization
                mSplashTimer = MAX(0.0f, kMinSplashTime - GameController::GC()->getContentLoadDuration());
            }
            else if (mSplashTimer > 0)
            {
                // Countdown
                mSplashTimer = MAX(0.0f, mSplashTimer - dt);
            }
            else
            {
                GLubyte opacity = mSplashProp->getOpacity();
                opacity = opacity >= 12 ? opacity - 12 : 0;
                mSplashProp->setOpacityChildren(opacity);
                if (opacity == 0)
                {
                    onEvent(EV_TYPE_SPLASH_DID_HIDE(), this);
                    
                    mSplashProp->removeFromParent();
                    mSplashProp = NULL, mSplashNode = NULL, mBgQuad = NULL;
                    GameController::GC()->destroySplashNodeResources();
                }
            }
        }
	}
}
