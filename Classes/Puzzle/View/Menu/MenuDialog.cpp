
#include "MenuDialog.h"
#include <Puzzle/View/Menu/MenuItem/GaugeMenuItem.h>
#include <Utils/FloatTweener.h>
#include <Puzzle/Inputs/TouchPad.h>
#include <Utils/Utils.h>
USING_NS_CC;

static const float kTouchPadding = 50.0f;

MenuDialog::MenuDialog(int category, int priority, uint inputFocus, NavigationMap navMap)
	:
	Prop(category),
	mCanGoBack(false),
	mPriority(priority),
    mShowScale(1.0f),
    mAnimState(ANIM_NONE),
    mButtonsProxy(inputFocus, navMap),
	mPressedItem(NULL),
	mBgContainer(NULL),
	mContentContainer(NULL),
	mMenuItemContainer(NULL),
	mListener(NULL),
    mCustomLayoutUid(-1),
    mCustomLayoutFunc(NULL),
    mCustomLayoutItems(NULL)
{
	
}

MenuDialog::~MenuDialog(void)
{
    for (std::vector<FloatTweener*>::iterator it = mAnimTweeners.begin(); it != mAnimTweeners.end(); ++it)
	{
        if (*it)
        {
            mScene->removeFromJuggler(*it);
            (*it)->setListener(NULL);
            CC_SAFE_RELEASE((*it));
        }
	}
    mAnimTweeners.clear();

	mPressedItem = NULL;
	for (std::set<MenuItem*>::iterator it = mMenuItems.begin(); it != mMenuItems.end(); ++it)
	{
		(*it)->setListener(NULL);
		CC_SAFE_RELEASE((*it));
	}
	mMenuItems.clear();
	mTouchableItems.clear();

	CC_SAFE_RELEASE_NULL(mBgContainer);
	CC_SAFE_RELEASE_NULL(mContentContainer);
	CC_SAFE_RELEASE_NULL(mMenuItemContainer);
    CC_SAFE_DELETE(mCustomLayoutItems);
    mCustomLayoutFunc = NULL;
}

MenuDialog* MenuDialog::create(int category, int priority, uint inputFocus, NavigationMap navMap, IEventListener* listener, bool autorelease)
{
	MenuDialog *dialog = new MenuDialog(category, priority, inputFocus, navMap);
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

bool MenuDialog::init(void)
{
	if (mBgContainer)
		mBgContainer->removeFromParent();
    else
        mBgContainer = new CCNode();
	addChild(mBgContainer, kZOrderBg);

	if (mContentContainer)
		mContentContainer->removeFromParent();
    else
        mContentContainer = new CCNode();
	addChild(mContentContainer, kZOrderContent);

	if (mMenuItemContainer)
		mMenuItemContainer->removeFromParent();
    else
        mMenuItemContainer = new CCNode();
	addChild(mMenuItemContainer, kZOrderMenuItem);
    
    mAnimTweeners.resize(ANIM_COUNT, NULL);
	
	return true;
}

void MenuDialog::show(AnimState anim)
{
    if (anim == ZOOM_OUT) // This is why we should have separate enums for show/hide
        anim = ANIM_NONE;
    
	setVisible(true);
    setAnimState(anim);
    
    if (anim != ZOOM_IN)
        setScale(mShowScale);
}

void MenuDialog::hide(AnimState anim)
{
    if (anim == ZOOM_IN || anim == DROP) // This is why we should have separate enums for show/hide
        anim = ANIM_NONE;
    
    if (anim == ANIM_NONE)
        setVisible(false);
    setPressedItem(NULL);
    setAnimState(anim);
}

void MenuDialog::setAnimState(AnimState value)
{
    if (value == mAnimState)
        return;
    
    // Clean up previous state
    AnimState prevState = mAnimState;
    
    switch (prevState)
    {
        case ANIM_NONE:
        case ANIM_COUNT:
            break;
        case DROP:
        {
            FloatTweener* animTweener = mAnimTweeners[prevState];
            if (animTweener)
                setPositionY(animTweener->getToValue());
        }
        // Fall through
        default:
        {
            FloatTweener* animTweener = mAnimTweeners[prevState];
            if (animTweener)
                mScene->removeFromJuggler(animTweener);
        }
            break;
    }
    
    // Apply new state
    mAnimState = value;
    
    switch (mAnimState)
    {
        case ZOOM_IN:
        {
            FloatTweener* animTweener = mAnimTweeners[mAnimState];
            if (animTweener == NULL)
            {
                animTweener = new FloatTweener(0, this, CMTransitions::EASE_OUT);
                mAnimTweeners[mAnimState] = animTweener;
            }
            else
                mScene->removeFromJuggler(animTweener);
            
            setScale(0);
            animTweener->reset(0, mShowScale, 0.3f);
            mScene->addToJuggler(animTweener);
        }
            break;
        case ZOOM_OUT:
        {
            FloatTweener* animTweener = mAnimTweeners[mAnimState];
            if (animTweener == NULL)
            {
                animTweener = new FloatTweener(0, this, CMTransitions::EASE_IN);
                mAnimTweeners[mAnimState] = animTweener;
            }
            else
                mScene->removeFromJuggler(animTweener);
            
            animTweener->reset(getScaleX(), 0, 0.15f);
            mScene->addToJuggler(animTweener);
        }
            break;
        case DROP:
        {
            FloatTweener* animTweener = mAnimTweeners[mAnimState];
            if (animTweener == NULL)
            {
                animTweener = new FloatTweener(0, this, CMTransitions::EASE_OUT_BACK);
                mAnimTweeners[mAnimState] = animTweener;
            }
            else
                mScene->removeFromJuggler(animTweener);
            
            float prevY = getPositionY();
            setPositionY(mScene->getViewHeight() + boundingBox().size.height);
            animTweener->reset(getPositionY(), prevY, 0.75f);
            mScene->addToJuggler(animTweener);
        }
            break;
        default:
            break;
    }
}

void MenuDialog::setShowScale(float scale)
{
    setScale(scale);
    mShowScale = MAX(1.0f, scale); // Game is unplayable if this is accidentally set too small
}

void MenuDialog::setPressedItem(MenuItem* item)
{
    if (mPressedItem)
        mPressedItem->reset();
    mPressedItem = item;
}

const char* MenuDialog::getMenuItemTextForTag(int tag)
{
	MenuItem* item = getMenuItem(tag);
	if (item)
		return item->getString();
	else
		return NULL;
}

void MenuDialog::setMenuItemTextForTag(int tag, const char* text)
{
	MenuItem* item = getMenuItem(tag);
	if (item && text)
		item->setString(text);
}

void MenuDialog::addBgItem(CCNode* item)
{
	if (item && mBgContainer)
	{
		mBgContainer->addChild(item);
		CCRect itemBounds = item->boundingBox();
		const CCSize& currentSize = getContentSize();
		setContentSize(CCSizeMake(MAX(currentSize.width, itemBounds.size.width), MAX(currentSize.height, itemBounds.size.height)));
	}
}

void MenuDialog::removeBgItem(CCNode* item)
{
	if (item && mBgContainer)
		mBgContainer->removeChild(item);
	// TODO: adjust content size if we ever actually need this.
}

void MenuDialog::addContentItem(CCNode* item)
{
	if (item && mContentContainer)
		mContentContainer->addChild(item);
}

void MenuDialog::removeContentItem(CCNode* item)
{
	if (item && mContentContainer)
		mContentContainer->removeChild(item);
}

void MenuDialog::addMenuItem(MenuItem* item)
{
	if (item && mMenuItems.count(item) == 0)
    {
        mButtonsProxy.addButton(item);
        if (mMenuItemContainer)
			mMenuItemContainer->addChild(item);
		mMenuItems.insert(item);
		item->setListener(this);
		item->retain();
    }
}

void MenuDialog::removeMenuItem(MenuItem* item)
{
	if (item && mMenuItems.count(item) != 0)
    {
        mButtonsProxy.removeButton(item);
        if (mMenuItemContainer)
			mMenuItemContainer->removeChild(item);
		if (mPressedItem && mPressedItem == item)
			setPressedItem(NULL);
		mMenuItems.erase(item);
		item->setListener(this);
		CC_SAFE_RELEASE(item);
    }
}

void MenuDialog::addTouchableItem(IEventListener* item)
{
    if (item && mTouchableItems.count(item) == 0)
        mTouchableItems.insert(item);
}

void MenuDialog::removeTouchableItem(IEventListener* item)
{
    if (item && mTouchableItems.count(item) != 0)
        mTouchableItems.erase(item);
}

MenuItem* MenuDialog::getMenuItem(int tag)
{
	for (std::set<MenuItem*>::iterator it = mMenuItems.begin(); it != mMenuItems.end(); ++it)
	{
		if ((*it)->getTag() == tag)
			return (*it);
	}

	return NULL;
}

MenuItem* MenuDialog::getMenuItem(const CCPoint& touchPos)
{
	CCPoint pos = this->convertToNodeSpace(touchPos);
	for (std::set<MenuItem*>::iterator it = mMenuItems.begin(); it != mMenuItems.end(); ++it)
	{
		CCRect itemBounds = CMUtils::boundsInSpace(this, *it);
		// Extend touch region downwards and slightly out to the sides. Users
		// generally touch lower than they intend to.
		itemBounds.setRect(
				itemBounds.origin.x - 10,
				itemBounds.origin.y - 20,
				itemBounds.size.width + 20,
				itemBounds.size.height + 20);
		if (itemBounds.containsPoint(pos))
			return *it;
	}
    
	return NULL;
}

MenuItem* MenuDialog::isPressedItem(const CCPoint& touchPos)
{
    if (mPressedItem != NULL)
    {
        CCPoint pos = this->convertToNodeSpace(touchPos);
        CCRect cancelBounds = CMUtils::boundsInSpace(this, mPressedItem);
        cancelBounds.setRect(
                             cancelBounds.origin.x - kTouchPadding,
                             cancelBounds.origin.y - kTouchPadding,
                             cancelBounds.size.width + 2 * kTouchPadding,
                             cancelBounds.size.height + 2 * kTouchPadding);
        if (cancelBounds.containsPoint(pos))
            return mPressedItem;
    }
    
    return getMenuItem(touchPos);
}

CCNode* MenuDialog::getContentItem(int tag)
{
    if (mContentContainer)
        return mContentContainer->getChildByTag(tag);
    return NULL;
}

void MenuDialog::addCustomLayoutItem(CCNode* item)
{
    if (item == NULL)
        return;
    
    if (mCustomLayoutItems == NULL)
        mCustomLayoutItems = new std::vector<CCNode*>();
    if (!CMUtils::contains(*mCustomLayoutItems, item))
        mCustomLayoutItems->push_back(item);
}

void MenuDialog::removeCustomLayoutItem(CCNode* item)
{
    if (item == NULL || mCustomLayoutItems == NULL)
        return;
    if (CMUtils::contains(*mCustomLayoutItems, item))
        CMUtils::erase(*mCustomLayoutItems, item);
}

void MenuDialog::layoutCustomItems(void)
{
    if (mCustomLayoutFunc && mCustomLayoutItems)
        mCustomLayoutFunc(mCustomLayoutUid, *mCustomLayoutItems);
}

bool MenuDialog::isAnimTweener(FloatTweener* tweener)
{
    if (tweener)
    {
        for (std::vector<FloatTweener*>::iterator it = mAnimTweeners.begin(); it != mAnimTweeners.end(); ++it)
        {
            if (tweener == *it)
                return true;
        }
    }
    
    return false;
}

void MenuDialog::resolutionDidChange(void)
{
    /* Do nothing */
}

void MenuDialog::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    for (std::set<MenuItem*>::iterator it = mMenuItems.begin(); it != mMenuItems.end(); ++it)
        (*it)->localeDidChange(fontKey, FXFontKey);
    
    if (mContentContainer)
    {
        CCArray* children = mContentContainer->getChildren();
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
    
    layoutCustomItems();
}

void MenuDialog::onEvent(int evType, void* evData)
{
	if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
        {
            if (tweener == mAnimTweeners[ZOOM_IN] || tweener == mAnimTweeners[ZOOM_OUT])
                setScale(tweener->getTweenedValue());
            else if (tweener == mAnimTweeners[DROP])
                setPositionY(tweener->getTweenedValue());
        }
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
        FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (isAnimTweener(tweener))
        {
            if (tweener == mAnimTweeners[ZOOM_OUT] && getAnimState() == ZOOM_OUT)
            {
                setVisible(false);
                this->retain();
                this->autorelease();
                removeFromParent();
            }
            
            setAnimState(ANIM_NONE);
        }
	}
	else if (evType == MenuItem::EV_TYPE_PRESSED())
	{
		MenuItem* item = static_cast<MenuItem*>(evData);
		CCAssert(mPressedItem == NULL, "MenuDialog: concurrently pressed items not permitted.");
		if (item)
			setPressedItem(item);
	}
	else if (evType == MenuItem::EV_TYPE_RAISED())
	{
		MenuItem* item = static_cast<MenuItem*>(evData);
        CCAssert(item == NULL || item == mPressedItem, "MenuDialog: concurrently pressed items not permitted.");
		if (item && item == mPressedItem)
		{
            setPressedItem(NULL);
            
            if (mListener)
            {
                int tag = item->getTag();
                mListener->onEvent(this->EV_TYPE_ITEM_DID_ACTIVATE(), &tag);
            }
		}	
	}
    else if (evType == GaugeMenuItem::EV_TYPE_GAUGE_CHANGED())
    {
        GaugeMenuItem* item = static_cast<GaugeMenuItem*>(evData);
        if (item)
        {
            int tag = item->getTag();
            mListener->onEvent(this->EV_TYPE_ITEM_DID_ACTIVATE(), &tag);
            
            if (item->shouldPlaySound())
                mScene->playSound("button");
        }
    }
	else if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN() || evType == TouchPad::EV_TYPE_TOUCH_MOVED() ||
		evType == TouchPad::EV_TYPE_TOUCH_ENDED() || evType == TouchPad::EV_TYPE_TOUCH_CANCELLED())
	{
		onTouch(evType, evData);
	}
}

void MenuDialog::onTouch(int evType, void* evData)
{
	CMTouches::TouchNotice* touchNotice = (CMTouches::TouchNotice*)evData;
	if (touchNotice == NULL)
		return;

	if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN())
	{
		MenuItem* item = getMenuItem(touchNotice->pos);
		if (item)
        {
            CCAssert(mPressedItem == NULL, "MenuDialog: concurrently pressed items not permitted.");
			item->depress();
        }
	}
	else if (evType == TouchPad::EV_TYPE_TOUCH_ENDED() || evType == TouchPad::EV_TYPE_TOUCH_CANCELLED())
	{
		MenuItem* item = isPressedItem(touchNotice->pos);
		if (item)
        {
            if (item == mPressedItem)
                item->raise();
            else
                item->reset();
        }
        
        if (mPressedItem)
            setPressedItem(NULL);
	}
	else if (evType == TouchPad::EV_TYPE_TOUCH_MOVED())
	{
		if (mPressedItem)
		{
			MenuItem* item = isPressedItem(touchNotice->pos);
			if (item == NULL || item != mPressedItem)
                setPressedItem(NULL);
		}
	}
    
    for (std::set<IEventListener*>::iterator it = mTouchableItems.begin(); it != mTouchableItems.end(); ++it)
        (*it)->onEvent(evType, evData);
}

void MenuDialog::update(int controllerState)
{
    mButtonsProxy.update(controllerState);
    
    for (std::set<MenuItem*>::iterator it = mMenuItems.begin(); it != mMenuItems.end(); ++it)
        (*it)->update(controllerState);
}

void MenuDialog::advanceTime(float dt)
{
#ifdef CHEEKY_DESKTOP
    mButtonsProxy.advanceTime(dt);
#endif
}

