
#include "Shield.h"
#include <Utils/ReusableCache.h>
#include <Utils/FloatTweener.h>
#include <Utils/ByteTweener.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Utils/Utils.h>
#include <Utils/Globals.h>
#include <Utils/PWDebug.h>
USING_NS_CC;

const float Shield::kShieldDomeRadius = 112.0f;
const float Shield::kShieldDomeDiameter = 2 * kShieldDomeRadius;
const GLubyte Shield::kDeployedAlpha = 215;

static const int kDeployAlphaTag = 1;
static const int kDeployScaleTag = 2;
static const int kWithdrawAlphaTag = 3;
static const int kWithdrawScaleTag = 4;

static const int kDeployIndex = 0;
static const int kWithdrawIndex = 1;

#pragma warning( disable : 4351 ) // new behavior: elements of array 'xyz' will be default initialized (changed in Visual C++ 2005)
Shield::Shield(int category, int id, int tileIndex)
	:
	Prop(category),
	mIsDeployed(false),
	mID(id),
    mTileIndex(tileIndex),
	mType(Shield::NORMAL),
	mElapsedTime(0),
    mStencilTextures(),
    mStencilRotations(),
	mShieldDome(NULL),
	mAlphaTweeners(),
	mScaleTweeners(),
	mListener(NULL)
{
    PWDebug::shieldCount++;
    mStencilTextures[0] = "shield-stencil-0";
    mStencilTextures[1] = "shield-stencil-0";
}

Shield::~Shield(void)
{
    mScene->removeTweensWithTarget(static_cast<IEventListener*>(this));
    
	for (int i = 0; i < 2; ++i)
	{
		if (mAlphaTweeners[i])
			CC_SAFE_RELEASE_NULL(mAlphaTweeners[i]);
		if (mScaleTweeners[i])
			CC_SAFE_RELEASE_NULL(mScaleTweeners[i]);
	}

	CC_SAFE_RELEASE_NULL(mShieldDome);

	mListener = NULL;
}

Shield* Shield::createWith(int category, int id, int tileIndex, const CCPoint& shieldOrigin, bool autorelease)
{
	Shield *shield = new Shield(category, id, tileIndex);
    if (shield && shield->initWith(shieldOrigin))
    {
		if (autorelease)
			shield->autorelease();
        return shield;
    }
    CC_SAFE_DELETE(shield);
    return NULL;
}

bool Shield::initWith(const CCPoint& shieldOrigin)
{
	bool bRet = true;
	setPosition(shieldOrigin);
	setVisible(false);

	do
	{
		if (mShieldDome == NULL)
		{
			mShieldDome = CCSprite::createWithTexture(mScene->textureByName("shield-dome"));
			if (!mShieldDome) { bRet = false; break; } else mShieldDome->retain();
			mShieldDome->setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey("Shield"));
			mShieldDome->setOpacity(0);
			addChild(mShieldDome);
		}

        CCRect bounds = mShieldDome->boundingBox();
		this->setContentSize(bounds.size);
	} while (false);

	return bRet;
}

void Shield::cancelTweens(void)
{
    mScene->removeTweensWithTarget(static_cast<IEventListener*>(this));
    
	for (int i = 0; i < 2; ++i)
	{
		if (mAlphaTweeners[i])
			mAlphaTweeners[i]->reset(0);
		if (mScaleTweeners[i])
			mScaleTweeners[i]->reset(0);
	}
}

void Shield::setType(ShieldType value)
{
	if (mType == value)
		return;

	switch (value)
	{
		case Shield::NORMAL:
			mShieldDome->setTexture(mScene->textureByName("shield-dome"));
			break;
		case Shield::TOP:
			mShieldDome->setTexture(mScene->textureByName("shield-dome-top"));
			break;
		case Shield::BOTTOM:
			mShieldDome->setTexture(mScene->textureByName("shield-dome-btm"));
			break;
	}

	mType = value;
}

void Shield::setStencil(int index, int texIndex, float texRotation)
{
    if (index >= 0 && index < 2 && texIndex >= 0 && texIndex < 5)
    {
        mStencilTextures[index] = CMUtils::strConcatVal("shield-stencil-", texIndex);
        mStencilRotations[index] = texRotation;
    }
}

void Shield::resetStencils(void)
{
    mStencilTextures[0] = "shield-stencil-0";
    mStencilTextures[1] = "shield-stencil-0";
    mStencilRotations[0] = mStencilRotations[1] = 0;
}

void Shield::deploy(bool playSound)
{
	if (isDeployed())
		return;

	setDeployed(true);
	mScene->removeTweensWithTarget(static_cast<IEventListener*>(this));
	notifyListener(EV_TYPE_DEPLOYING());
	setScale(0.0f);
	setVisible(true);

	if (mAlphaTweeners[kDeployIndex] == NULL || mScaleTweeners[kDeployIndex] == NULL)
	{
		mAlphaTweeners[kDeployIndex] = new ByteTweener(0, this, CMTransitions::EASE_OUT_BACK);
		mAlphaTweeners[kDeployIndex]->setTag(kDeployAlphaTag);
		mScaleTweeners[kDeployIndex] = new FloatTweener(0, this, CMTransitions::EASE_OUT_BACK);
		mScaleTweeners[kDeployIndex]->setTag(kDeployScaleTag);
	}

	float duration = 0.4f;
	mAlphaTweeners[kDeployIndex]->reset(mShieldDome->getOpacity(), kDeployedAlpha, duration);
	mScaleTweeners[kDeployIndex]->reset(getScale(), 1.0f, duration);
	mScene->addToJuggler(mAlphaTweeners[kDeployIndex]);
	mScene->addToJuggler(mScaleTweeners[kDeployIndex]);
    if (playSound)
        mScene->playSound("tile-shield-activate");
}

void Shield::withdraw(bool playSound)
{
	if (!isDeployed())
		return;

	setDeployed(false);
	mScene->removeTweensWithTarget(static_cast<IEventListener*>(this));

	if (mAlphaTweeners[kWithdrawIndex] == NULL || mScaleTweeners[kWithdrawIndex] == NULL)
	{
		mAlphaTweeners[kWithdrawIndex] = new ByteTweener(0, this, CMTransitions::EASE_OUT);
		mAlphaTweeners[kWithdrawIndex]->setTag(kWithdrawAlphaTag);
		mScaleTweeners[kWithdrawIndex] = new FloatTweener(0, this, CMTransitions::EASE_OUT);
		mScaleTweeners[kWithdrawIndex]->setTag(kWithdrawScaleTag);
	}

	float duration = 0.4f;
	mAlphaTweeners[kWithdrawIndex]->reset(mShieldDome->getOpacity(), 0, duration);
	mScaleTweeners[kWithdrawIndex]->reset(getScale(), 0, duration);
	mScene->addToJuggler(mAlphaTweeners[kWithdrawIndex]);
	mScene->addToJuggler(mScaleTweeners[kWithdrawIndex]);

	if (playSound)
		mScene->playSound("tile-shield-deactivate");
	notifyListener(EV_TYPE_WITHDRAWING());
}

void Shield::notifyListener(uint evCode)
{
	if (mListener)
		mListener->onEvent(evCode, this);
}

void Shield::onEvent(int evType, void* evData)
{
	if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
			onTweenerChanged(tweener->getTag(), 0, tweener->getTweenedValue());
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
			onTweenerChanged(tweener->getTag(), tweener->getTweenedValue(), 0);
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
			onTweenerCompleted(tweener->getTag(), 0, tweener->getTweenedValue());

	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
			onTweenerCompleted(tweener->getTag(), tweener->getTweenedValue(), 0);
	}
}

void Shield::onTweenerChanged(int tag, GLubyte bVal, float fVal)
{
	switch (tag)
	{
		case kDeployAlphaTag:
		case kWithdrawAlphaTag:
			mShieldDome->setOpacity(bVal);
			break;
		case kDeployScaleTag:
		case kWithdrawScaleTag:
			setScale(fVal);
			break;
	}
}

void Shield::onTweenerCompleted(int tag, GLubyte bVal, float fVal)
{
	switch (tag)
	{
		case kDeployAlphaTag:
			notifyListener(EV_TYPE_DEPLOYED());
			break;
		case kWithdrawAlphaTag:
			setVisible(false);
			notifyListener(EV_TYPE_WITHDRAWN());
			break;
	}
}
   
void Shield::advanceTime(float dt)
{
	mElapsedTime += dt;
}

void Shield::visit(void)
{
	if (mShieldDome)
	{
		CCGLProgram* glProgram = mShieldDome->getShaderProgram();
		glProgram->use();

		CCPoint displacementScroll = CMUtils::moveInCircle(mElapsedTime, 0.15f);
		glUniform2f(mScene->uniformLocationByName("u_displacementScrollS"), displacementScroll.x, displacementScroll.y);
		
        glUniform1f(mScene->uniformLocationByName("u_stencilRotation0S"), mStencilRotations[0]);
        glUniform1f(mScene->uniformLocationByName("u_stencilRotation1S"), mStencilRotations[1]);
        
		GLuint texId = mScene->textureByName("plasma")->getName();
		ccGLBindTexture2DN(mScene->textureUnitById(texId), texId);
        
        texId = mScene->textureByName("sin2x-table")->getName();
		ccGLBindTexture2DN(mScene->textureUnitById(texId), texId);
        
        texId = mScene->textureByName(mStencilTextures[0].c_str())->getName();
		ccGLBindTexture2DN(6, texId);
        
        texId = mScene->textureByName(mStencilTextures[1].c_str())->getName();
		ccGLBindTexture2DN(7, texId);
	}

	Prop::visit();
}

/* Reusable Implementation */
ReusableCache* Shield::sCache = NULL;
bool Shield::sCaching = false;

void Shield::setupReusables(void)
{
	if (sCache)
        return;

	sCaching = true;
    sCache = new ReusableCache();

#if CM_SMALL_CACHES
    int cacheSize = 8;
#else
    int cacheSize = 25;
#endif
    uint reuseKey = 0;
    IReusable* reusable = NULL;
    PuzzleTile tile;
    sCache->addKey(cacheSize, reuseKey);

	CCPoint origin = ccp(0, 0);
    for (int i = 0; i < cacheSize; ++i)
    {
        reusable = getShield(i, 0, origin);
        reusable->hibernate();
        sCache->addReusable(reusable);
    }

	sCache->verifyCacheIntegrity();
    sCaching = false;
}

Shield* Shield::getShield(int id, int tileIndex, const CCPoint& shieldOrigin)
{
	uint reuseKey = 0;
    Shield* shield = static_cast<Shield*>(checkoutReusable(reuseKey));

    if (shield)
    {
        shield->reuse();
        shield->reconfigure(id, tileIndex, shieldOrigin);
    }
    else
    {
		shield = Shield::createWith(CMGlobals::BOARD, id, tileIndex, shieldOrigin, !sCaching);
    }

    return shield;
}

IReusable* Shield::checkoutReusable(uint reuseKey)
{
	IReusable* reusable = NULL;

    if (sCache && !sCaching)
		reusable = sCache->checkout(reuseKey);

    return reusable;
}

void Shield::checkinReusable(IReusable* reusable)
{
	if (sCache && !sCaching && reusable)
		sCache->checkin(reusable);
}
	
void Shield::reuse(void)
{
	if (getInUse())
        return;

	if (mShieldDome)
		mShieldDome->setOpacity(0);
	setDeployed(false);
	setVisible(false);
	setScale(1);

    mInUse = true;
}

void Shield::hibernate(void)
{
	if (!getInUse())
        return;

	cancelTweens();
	removeFromParent();
	setListener(NULL);

    mInUse = false;
    checkinReusable(this);
}

void Shield::reconfigure(int id, int tileIndex, const CCPoint& shieldOrigin)
{
	mID = id;
    mTileIndex = tileIndex;
	setPosition(shieldOrigin);
    resetStencils();
}

void Shield::returnToPool(void)
{
    if (getPoolIndex() != -1)
        hibernate();
    else
        removeFromParent();
}
