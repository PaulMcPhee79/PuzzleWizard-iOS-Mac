
#include "PlayerHUD.h"
#include <Puzzle/Data/Player.h>
#include <Puzzle/View/Playfield/HUD/HUDCell.h>
#include <Utils/ByteTweener.h>
#include <Utils/Utils.h>
#include <Utils/LangFX.h>
USING_NS_CC;

static const float kTransitionInDuration = 0.25f;
static const float kTransitionOutDuration = 0.25f;
static const float kBgOpacityMax = 0.7f;

static const char* kHudStringSingular = "? move remaining";
static const char* kHudStringPlural = "? moves remaining";

PlayerHUD::PlayerHUD(int category, const CCRect& bounds)
	:
	Prop(category),
	mOriginBounds(bounds),
	mBgQuad(NULL),
	mTransitionTweener(NULL),
    mPrevPlayerValue(10),
	mState(PlayerHUD::HS_IN),
	mPlayer(NULL),
	mHUDCell(NULL)
{
	mAdvanceable = true;
}

PlayerHUD::~PlayerHUD(void)
{
    mScene->deregisterLocalizable(this);
    
	if (mPlayer)
		mPlayer->registerView(this);
	CC_SAFE_RELEASE_NULL(mPlayer);

	if (mTransitionTweener)
		mTransitionTweener->setListener(NULL);
	CC_SAFE_RELEASE_NULL(mTransitionTweener);
}

PlayerHUD* PlayerHUD::create(int category, const CCRect& bounds, bool autorelease)
{
	PlayerHUD *hud = new PlayerHUD(category, bounds);
	if (hud && hud->init())
    {
		if (autorelease)
			hud->autorelease();
        return hud;
    }
    CC_SAFE_DELETE(hud);
    return NULL;
}

bool PlayerHUD::init(void)
{
	if (mBgQuad)
		return true;

	mBgQuad = CMUtils::createColoredQuad(CCSizeMake(mOriginBounds.size.width, mOriginBounds.size.height));
	mBgQuad->setColor(CMUtils::uint2color3B(0x404040));
	mBgQuad->setOpacity((GLubyte)(kBgOpacityMax * 255));
	addChild(mBgQuad);

	mHUDCell = HUDCell::create();
	mHUDCell->setPositionX(-mOriginBounds.size.width / 2);
	addChild(mHUDCell);

	mTransitionTweener = new ByteTweener(0, this, CMTransitions::LINEAR);
	setState(PlayerHUD::HS_OUT);

	this->setContentSize(mBgQuad->boundingBox().size);
    mScene->registerLocalizable(this);

	return true; // Obvious when it fails
}

void PlayerHUD::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    if (mHUDCell)
    {
        mHUDCell->localeDidChange(fontKey, FXFontKey);
        mHUDCell->setString(getHudString(mPrevPlayerValue).c_str());
    }
    
    if (mBgQuad && mHUDCell)
    {
        mOriginBounds = CCRectMake(mOriginBounds.origin.x, mOriginBounds.origin.y, LangFX::getPlayerHUDSettings().val_1, mOriginBounds.size.height);
        mBgQuad->setTextureRect(CCRectMake(0, 0, mOriginBounds.size.width, mOriginBounds.size.height));
        mHUDCell->setPositionX(-mOriginBounds.size.width / 2);
    }
}

void PlayerHUD::setState(HUDState value)
{
	switch (value)
    {
        case PlayerHUD::HS_OUT:
            setOpacityChildren(0);
            setVisible(false);
            mTransitionTweener->reset(getOpacity());
            break;
        case PlayerHUD::HS_TRANSITION_IN:
            mTransitionTweener->reset(getOpacity(), 255, kTransitionInDuration);
            setVisible(true);
            break;
        case PlayerHUD::HS_IN:
			mBgQuad->setOpacity((GLubyte)(kBgOpacityMax * 255));
            mHUDCell->setOpacityChildren(255);
			setOpacity(255);
            mTransitionTweener->reset(getOpacity());
            break;
        case PlayerHUD::HS_TRANSITION_OUT:
            mTransitionTweener->reset(getOpacity(), 0, kTransitionOutDuration);
            break;
    }

    mState = value;
}

void PlayerHUD::setPlayer(Player* value)
{
	if (mPlayer == value)
		return;

	if (mPlayer)
        mPlayer->deregisterView(this);

	CC_SAFE_RETAIN(value);
	CC_SAFE_RELEASE(mPlayer);
    mPlayer = value;

    if (mPlayer)
        mPlayer->registerView(this);

    if (getState() == PlayerHUD::HS_IN || getState() == PlayerHUD::HS_TRANSITION_IN)
        setState(PlayerHUD::HS_TRANSITION_OUT);
}

std::string PlayerHUD::getHudString(int value) const
{
    std::string hudString = value == 1 ? SceneController::localizeString(kHudStringSingular) : SceneController::localizeString(kHudStringPlural);
    size_t tokenPos = hudString.find_first_of("?");
    if (tokenPos != std::string::npos)
    {
        std::string replaceStr = CMUtils::strConcatVal("", value);
        hudString.replace(tokenPos, 1, replaceStr);
    }
	return hudString;
}

uint PlayerHUD::getTextColorForValue(int value, int range)
{
    if (MODE_8x6)
    {
        if (value >= 0.74f * range)
            return kHUDGreen;
        else if (value >= 0.374f * range)
            return kHUDAmber;
        else
            return kHUDRed;
    }
    else
    {
        if (value >= 0.69f * range)
            return kHUDGreen;
        else if (value >= 0.25f * range)
            return kHUDAmber;
        else
            return kHUDRed;
    }
}

void PlayerHUD::playerValueDidChange(uint code, int value)
{
	switch (code)
    {
		case Player::kValueProperty:
            return;
        case Player::kValueColorMagic:
            {
				mHUDCell->setIcon(code);
				mHUDCell->setString(getHudString(value).c_str());
                uint textColor = MODE_8x6 ? getTextColorForValue(value, 4) : getTextColorForValue(value, 5);
				mHUDCell->setColor(textColor);
            }
            break;
        case Player::kValueMirroredMate:
            {
                mHUDCell->setIcon(code);
                mHUDCell->setString(getHudString(value).c_str());
                uint textColor = MODE_8x6 ? getTextColorForValue(value, 8) : getTextColorForValue(value, 10);
				mHUDCell->setColor(textColor);
            }
            break;
    }

	HUDState state = getState();
    if (value == 0 && (state == PlayerHUD::HS_IN || state == PlayerHUD::HS_TRANSITION_IN))
        setState(PlayerHUD::HS_TRANSITION_OUT);
    else if (value > 0 && (state == PlayerHUD::HS_OUT || state == PlayerHUD::HS_TRANSITION_OUT))
        setState(PlayerHUD::HS_TRANSITION_IN);
    
    mPrevPlayerValue = value;
}

void PlayerHUD::onEvent(int evType, void* evData)
{
	if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		float tweenedValue = mTransitionTweener->getTweenedValue();
		mBgQuad->setOpacity((GLubyte)(kBgOpacityMax * tweenedValue));
		mHUDCell->setOpacityChildren(tweenedValue);
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
		if (getState() == PlayerHUD::HS_TRANSITION_IN)
			setState(PlayerHUD::HS_IN);
		else if (getState() == PlayerHUD::HS_TRANSITION_OUT)
			setState(PlayerHUD::HS_OUT);
	}
}

void PlayerHUD::advanceTime(float dt)
{
	if (mTransitionTweener && (getState() == PlayerHUD::HS_TRANSITION_IN || getState() == PlayerHUD::HS_TRANSITION_OUT))
		mTransitionTweener->advanceTime(dt);
}
