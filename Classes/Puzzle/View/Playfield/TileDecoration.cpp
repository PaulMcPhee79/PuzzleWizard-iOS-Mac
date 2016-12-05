
#include "TileDecoration.h"
#include <Utils/ReusableCache.h>
#include <Utils/ByteTweener.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Interfaces/ITileDecorator.h>
#include <Extensions/MovieClip.h>
#include <Puzzle/View/Playfield/Effects/Twinkle.h>
#include <Utils/Utils.h>
#include <Puzzle/View/PuzzleHelper.h>
#include <Utils/PWDebug.h>
USING_NS_CC;

const GLubyte TileDecoration::kTeleportGlowOpacityMin = 96;
const GLubyte TileDecoration::kTeleportGlowOpacityMax = 255;
const float TileDecoration::kTeleportGlowDuration = 1.25f;
const float TileDecoration::kColorArrowFps = 36.0f;

static const uint kColorArrowColor = 0xffffff;
static const float kKeyTwinkleDuration = 1.0f;

TileDecoration::TileDecoration(uint type, uint subType)
	:
mType(type),
mSubType(subType),
mUIModeEnabled(false),
mDecorator(NULL),
mNormalShader(NULL),
mKeyShader(NULL),
mTwinkle(NULL)
{
    PWDebug::tileDecorationCount++;
}

TileDecoration::~TileDecoration(void)
{
	setDecorator(NULL);
    enableUIMode(false);

	if (mTwinkle != NULL)
	{
		mTwinkle->returnToPool();
		mTwinkle->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mTwinkle);
	}

	if (isAdvanceable())
		mScene->removeFromJuggler(this);
}

TileDecoration* TileDecoration::create(uint type, uint subType, bool autorelease)
{
	TileDecoration *decoration = new TileDecoration(type, subType);
    if (decoration && decoration->init())
    {
		if (autorelease)
			decoration->autorelease();
        return decoration;
    }
    CC_SAFE_DELETE(decoration);
    return NULL;
}

bool TileDecoration::init(void)
{
	bool bRet = CMSprite::init();
	if (!bRet) return false;

	CCSpriteFrame* boundingFrame = NULL;
    switch (mType)
    {
		case TilePiece::kTDKTeleport:
            {
				std::string textureName = CMUtils::strConcatVal("teleport-rune-", mSubType);
				setDisplayFrame(mScene->spriteFrameByName(textureName.c_str()));
				
				std::string glowTextureName = CMUtils::strConcatVal("teleport-glow-", mSubType);
				CCSprite* glowIcon = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName(glowTextureName.c_str()));
				if (!(bRet = bRet && glowIcon)) break;
				glowIcon->setPosition(ccp(glowIcon->boundingBox().size.width / 2, glowIcon->boundingBox().size.height / 2));
				glowIcon->setOpacity(kTeleportGlowOpacityMax);
				glowIcon->setTag((int)TilePiece::kTDKTeleport);
				addChild(glowIcon);

                setAnimatableName("TileDecoration::kTDKTeleport");
				setAdvanceable(true);
            }
            break;
        case TilePiece::kTDKColorSwap:
            {
                uint colorLeft, colorRight;
				Puzzles::setColorSwapColorsForTile(mSubType, colorLeft, colorRight);
				setDisplayFrame(mScene->spriteFrameByName("color-swap-glyph-left"));
				setColor(CMUtils::uint2color3B(colorLeft));
				CCSprite* rightGlyph = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("color-swap-glyph-right"));
				if (!(bRet = bRet && rightGlyph)) break;
				if (rightGlyph)
				{
					rightGlyph->setAnchorPoint(ccp(0.0, 0.0));
					rightGlyph->setTag((int)TilePiece::kTDKColorSwap);
					rightGlyph->setColor(CMUtils::uint2color3B(colorRight));
					addChild(rightGlyph);
				}
            }
            break;
        case TilePiece::kTDKRotate:
            {
				setDisplayFrame(mScene->spriteFrameByName("rotate"));
            }
            break;
        case TilePiece::kTDKShield:
            {
				setDisplayFrame(mScene->spriteFrameByName("shield"));
            }
            break;
        case TilePiece::kTDKPainter:
            {
				setDisplayFrame(mScene->spriteFrameByName("clear"));
				CCArray* streakFrames = mScene->spriteFramesStartingWith("color-streak_");
                for (int i = 0; i < 4 && bRet; ++i)
                {
                    uint colorKey = (mSubType >> (i * 4)) & PuzzleTile::kColorKeyMask;
					CCSprite* colorArrow = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("color-arrow"));
					if (!(bRet = bRet && colorArrow)) break;
					colorArrow->setTag(2 * i);
					colorArrow->setColor(CMUtils::uint2color3B(kColorArrowColor));
					colorArrow->setRotation(i * 90);
					colorArrow->setVisible(colorKey != 0);

					if (boundingFrame == NULL)
						boundingFrame = colorArrow->displayFrame();

					MovieClip* colorStreak = MovieClip::createWithFrames(streakFrames, TileDecoration::kColorArrowFps);
					if (!(bRet = bRet && colorStreak)) break;
					colorStreak->setTag(2 * i + 1);
					colorStreak->setColor(CMUtils::uint2color3B(Puzzles::colorForKey(colorKey)));
					colorStreak->setRotation(i * 90);
					colorStreak->setVisible(colorKey != 0);
                    colorStreak->setAnimatableName("TileDecoration::MovieClip::ColorStreak");

                    //if (colorArrow->isVisible())
					//	mScene->addToJuggler(colorStreak);
                    addChild(colorStreak);
                    addChild(colorArrow);
                }
                
                setAnimatableName("TileDecoration::kTDKPainter");
                setAdvanceable(true);
            }
            break;
        case TilePiece::kTDKTileSwap:
            {
				std::string textureName = CMUtils::strConcatVal("tile-swap-", mSubType);
				setDisplayFrame(mScene->spriteFrameByName(textureName.c_str()));
            }
            break;
        case TilePiece::kTDKMirroredImage:
            {
				setDisplayFrame(mScene->spriteFrameByName("mirrored"));
            }
            break;
        case TilePiece::kTDKKey:
            {
				setDisplayFrame(mScene->spriteFrameByName("key"));

				if (mTwinkle == NULL)
				{
					CCSprite* twinkleScaler = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("clear"));
					twinkleScaler->setPosition(ccp(
						0.75f * boundingBox().size.width,
						0.725f * boundingBox().size.height));
					twinkleScaler->setScale(0.5f);
					addChild(twinkleScaler);

					mTwinkle = Twinkle::getTwinkle(CCPointZero);
					mTwinkle->retain();
					twinkleScaler->addChild(mTwinkle);
				}

				mNormalShader = getShaderProgram();
				mKeyShader = CCShaderCache::sharedShaderCache()->programForKey("SparkleGradient");
				setShaderProgram(mKeyShader);
            }
            break;
        case TilePiece::kTDKColorFill:
            {
				setDisplayFrame(mScene->spriteFrameByName("color-flood"));
				setColor(CMUtils::uint2color3B(Puzzles::colorForKey(mSubType & PuzzleTile::kColorKeyMask)));
            }
            break;
        case TilePiece::kTDKPaintBomb:
            {
				setDisplayFrame(mScene->spriteFrameByName("color-swirl"));
            }
            break;
        case TilePiece::kTDKConveyorBelt:
            {
				setDisplayFrame(mScene->spriteFrameByName("conveyor-belt"));
				setRotation((int)mSubType * 90);
            }
            break;
        case TilePiece::kTDKColorMagic:
            {
				setDisplayFrame(mScene->spriteFrameByName("color-magic"));
            }
            break;
        default:
            break;
    }

	if (boundingFrame == NULL)
		boundingFrame = this->displayFrame();
	if (boundingFrame)
	{
		const CCSize& size = boundingFrame->getOriginalSize();
		setPosition(ccp(size.width / 2, size.height / 2));
	}
    
    if (!sCaching && isAdvanceable())
        mScene->addToJuggler(this);

	return bRet;
}

void TileDecoration::setSubType(uint value)
{
	// Must always re-add these to juggler regardless of same subtype.
	if (getType() == TilePiece::kTDKPainter)
	{
		for (int i = 0; i < 4; ++i)
		{
			uint colorKey = (value >> (i * 4)) & PuzzleTile::kColorKeyMask;

			CCSprite* colorArrow = static_cast<CCSprite*>(getChildByTag(2 * i));
			colorArrow->setColor(CMUtils::uint2color3B(kColorArrowColor));
			colorArrow->setVisible(colorKey != 0);

			MovieClip* colorStreak = static_cast<MovieClip*>(getChildByTag(2 * i + 1));
			//mScene->removeFromJuggler(colorStreak);
			colorStreak->setColor(CMUtils::uint2color3B(Puzzles::colorForKey(colorKey)));
			colorStreak->setVisible(colorKey != 0);

			//if (colorArrow->isVisible())
			//	mScene->addToJuggler(colorStreak);
		}
	}
	else if (getType() == TilePiece::kTDKTeleport)
	{
		std::string textureName = CMUtils::strConcatVal("teleport-rune-", value);
		setDisplayFrame(mScene->spriteFrameByName(textureName.c_str()));
		
		CCSprite* glowIcon = static_cast<CCSprite*>(getChildByTag((int)TilePiece::kTDKTeleport));
		if (glowIcon)
		{
			std::string glowTextureName = CMUtils::strConcatVal("teleport-glow-", value);
			glowIcon->setDisplayFrame(mScene->spriteFrameByName(glowTextureName.c_str()));
			glowIcon->setOpacity(kTeleportGlowOpacityMax);
		}
	}

	// Don't break early - it causes unforeseen problems (like intermittent Color Blind bugs)
	//if (mSubType == value)
	//	return;

	switch (getType())
    {
        case TilePiece::kTDKColorSwap:
            {
                uint colorLeft, colorRight;
				Puzzles::setColorSwapColorsForTile(value, colorLeft, colorRight);
				setColor(CMUtils::uint2color3B(colorLeft));
				static_cast<CCSprite*>(getChildByTag((int)TilePiece::kTDKColorSwap))->setColor(CMUtils::uint2color3B(colorRight));
            }
            break;
        case TilePiece::kTDKTileSwap:
            {
				std::string textureName = CMUtils::strConcatVal("tile-swap-", value);
				setDisplayFrame(mScene->spriteFrameByName(textureName.c_str()));
            }
            break;
        case TilePiece::kTDKColorFill:
            {
				setColor(CMUtils::uint2color3B(Puzzles::colorForKey(value & PuzzleTile::kColorKeyMask)));
            }
            break;
        case TilePiece::kTDKConveyorBelt:
            {
				setRotation((int)value * 90);
            }
            break;
        default:
            break;
    }

	mSubType = value;
}

void TileDecoration::setDecorator(ITileDecorator* value)
{
	mDecorator = value;

    if (getDecorator())
    {
		setTeleportOpacity(getDecorator()->decoratorValueForKey(getType()));
        setColorArrowFrameIndex((int)getDecorator()->decoratorValueForKey(getType()));
    }
}

GLubyte TileDecoration::getTeleportOpacity(void)
{
	GLubyte opacity = 255;

	if (getType() == TilePiece::kTDKTeleport)
	{
		CCSprite* teleportGlow = static_cast<CCSprite*>(getChildByTag((int)TilePiece::kTDKTeleport));
		if (teleportGlow)
			opacity = teleportGlow->getOpacity();
	}

	return opacity;
}

void TileDecoration::setTeleportOpacity(GLubyte value)
{
	if (getType() == TilePiece::kTDKTeleport)
	{
		CCSprite* teleportGlow = static_cast<CCSprite*>(getChildByTag((int)TilePiece::kTDKTeleport));
		if (teleportGlow)
			teleportGlow->setOpacity(value);
	}
}

void TileDecoration::setColorArrowFrameIndex(int value)
{
    if (getType() == TilePiece::kTDKPainter)
	{
		for (int i = 0; i < 4; ++i)
        {
            MovieClip* colorStreak = static_cast<MovieClip*>(this->getChildByTag(2 * i + 1));
            if (colorStreak)
                colorStreak->setCurrentFrame(value);
        }
	}
}

void TileDecoration::enableUIMode(bool enable)
{
    if (getType() == TilePiece::kTDKPainter)
        mUIModeEnabled = enable;
//	{
//		for (int i = 0; i < 4; ++i)
//        {
//            MovieClip* colorStreak = static_cast<MovieClip*>(this->getChildByTag(2 * i + 1));
//            if (colorStreak)
//            {
//                if (enable)
//                    mScene->addToJuggler(colorStreak);
//                else
//                    mScene->removeFromJuggler(colorStreak);
//            }
//        }
//	}
}

void TileDecoration::enableMenuMode(bool enable)
{
	switch (getType())
    {
		case TilePiece::kTDKTeleport:
			{
				CCSprite* glowIcon = static_cast<CCSprite*>(getChildByTag((int)TilePiece::kTDKTeleport));
				if (glowIcon)
                {
                    //std::string glowTextureName = CMUtils::strConcatVal(enable ? "mini-teleport-glow-" : "teleport-glow-", getSubType());
                    //glowIcon->setDisplayFrame(mScene->spriteFrameByName(glowTextureName.c_str()));
					glowIcon->setOpacity(kTeleportGlowOpacityMax);
                }
				if (enable)
					mScene->removeFromJuggler(this);
				else
					mScene->addToJuggler(this);
			}
			break;
        case TilePiece::kTDKColorSwap:
            {
                setDisplayFrame(mScene->spriteFrameByName(enable ? "mini-color-swap-glyph-left" : "color-swap-glyph-left"));
                static_cast<CCSprite*>(getChildByTag((int)TilePiece::kTDKColorSwap))->setDisplayFrame(
                    mScene->spriteFrameByName(enable ? "mini-color-swap-glyph-right" : "color-swap-glyph-right"));
            }
            break;
        case TilePiece::kTDKRotate:
            {
                setDisplayFrame(mScene->spriteFrameByName(enable ? "mini-rotate" : "rotate"));
            }
            break;
        case TilePiece::kTDKShield:
            {
                setDisplayFrame(mScene->spriteFrameByName(enable ? "mini-shield" : "shield"));
            }
            break;
        case TilePiece::kTDKPainter:
            {
                for (int i = 0; i < 4; ++i)
                {
                    uint colorKey = (mSubType >> (i * 4)) & PuzzleTile::kColorKeyMask;
                    CCSprite* colorArrow = static_cast<CCSprite*>(getChildByTag(2 * i));
                    colorArrow->setDisplayFrame(mScene->spriteFrameByName(enable ? "mini-painter" : "color-arrow"));
                    
                    if (enable)
                        colorArrow->setColor(CMUtils::uint2color3B(Puzzles::colorForKey(colorKey)));
                    else
                        colorArrow->setColor(CMUtils::uint2color3B(kColorArrowColor));
                    static_cast<MovieClip*>(getChildByTag(2 * i + 1))->setVisible(!enable && colorArrow->isVisible());
                }
                if (enable)
					mScene->removeFromJuggler(this);
				else
					mScene->addToJuggler(this);
            }
            break;
        case TilePiece::kTDKTileSwap:
            {
                std::string textureName = CMUtils::strConcatVal(enable ? "mini-tile-swap-" : "tile-swap-", mSubType);
                setDisplayFrame(mScene->spriteFrameByName(textureName.c_str()));
            }
            break;
        case TilePiece::kTDKMirroredImage:
            {
                setDisplayFrame(mScene->spriteFrameByName(enable ? "mini-mirrored" : "mirrored"));
            }
            break;
		case TilePiece::kTDKKey:
			{
                setDisplayFrame(mScene->spriteFrameByName(enable ? "mini-key" : "key"));
				if (mNormalShader && mKeyShader)
					setShaderProgram((enable ? mNormalShader : mKeyShader));
			}
			break;
        case TilePiece::kTDKColorFill:
            {
                setDisplayFrame(mScene->spriteFrameByName(enable ? "mini-color-flood" : "color-flood"));
            }
            break;
        case TilePiece::kTDKPaintBomb:
            {
                setDisplayFrame(mScene->spriteFrameByName(enable ? "mini-color-swirl" : "color-swirl"));
            }
            break;
        case TilePiece::kTDKConveyorBelt:
            {
                setDisplayFrame(mScene->spriteFrameByName(enable ? "mini-conveyor-belt" : "conveyor-belt"));
            }
            break;
        case TilePiece::kTDKColorMagic:
            {
                setDisplayFrame(mScene->spriteFrameByName(enable ? "mini-color-magic" : "color-magic"));
            }
            break;
	}
}

void TileDecoration::syncWithTileDecoration(TileDecoration* tileDecoration)
{
	if (tileDecoration == NULL || getType() != tileDecoration->getType())
        return;

    switch (getType())
    {
		case TilePiece::kTDKTeleport:
            {
                setTeleportOpacity(tileDecoration->getTeleportOpacity());
            }
            break;
//		case TilePiece::kTDKPainter:
//			{
//				for (int i = 0; i < 4; ++i)
//				{
//					MovieClip* colorStreak = static_cast<MovieClip*>(this->getChildByTag(2 * i + 1));
//					MovieClip* otherColorStreak = static_cast<MovieClip*>(tileDecoration->getChildByTag(2 * i + 1));
//					if (colorStreak && otherColorStreak)
//						colorStreak->setCurrentFrame(otherColorStreak->getCurrentFrame());
//				}
//			}
//			break;
		case TilePiece::kTDKKey:
            {
				mSparkleX = tileDecoration->mSparkleX;
                if (mTwinkle && tileDecoration->mTwinkle)
                    mTwinkle->syncWithTwinkle(tileDecoration->mTwinkle);
            }
            break;
    }
}

void TileDecoration::advanceTime(float dt)
{
	CCAssert(getType() == TilePiece::kTDKTeleport || getType() == TilePiece::kTDKPainter, "Bad state in TileDecoration::advanceTime.");

	if (getDecorator())
    {
		setTeleportOpacity(getDecorator()->decoratorValueForKey(getType()));
        setColorArrowFrameIndex((int)getDecorator()->decoratorValueForKey(getType()));
    }
    else if (mUIModeEnabled && getType() == TilePiece::kTDKPainter && isVisible())
    {
        for (int i = 0; i < 4; ++i)
        {
            MovieClip* colorStreak = static_cast<MovieClip*>(this->getChildByTag(2 * i + 1));
            if (colorStreak && colorStreak->isVisible())
                colorStreak->advanceTime(dt);
        }
    }
}

void TileDecoration::draw(void)
{
    if (getBatchNode() || !isVisible())
        return;
    
	uint type = getType();

	switch (type)
    {
		case TilePiece::kTDKKey:
			{
				ITileDecorator* decorator = getDecorator();
				if (decorator && mKeyShader && getShaderProgram() == mKeyShader)
				{
					mSparkleX = decorator->decoratorValueForKey(type);

					CCGLProgram* glProgram = mKeyShader;
					glProgram->use();

					// 255 / 6.0f = 42.5f -> converts from byte range to seconds range (6 second loop).
					float sparkleX = mSparkleX / 42.5f;

					glUniform2f(mScene->uniformLocationByName("u_gradientCoordSG"), (sparkleX < 1.0f ? -0.75f * sparkleX : 0.0f), 0.0f);
					GLuint texId = mScene->textureByName("sparkle-gradient")->getName();
					ccGLBindTexture2DN(mScene->textureUnitById(texId), texId);

					if (mTwinkle && !mTwinkle->isAnimating() && sparkleX >= 0.4f && sparkleX < (0.4f + 0.8f * kKeyTwinkleDuration))
					{
						mTwinkle->animate(kKeyTwinkleDuration);
						mTwinkle->fastForward(sparkleX - 0.4f);
					}
				}
			}
			break;
	}
	
	CMSprite::draw();
}

/* Reusable Implementation */
ReusableCache* TileDecoration::sCache = NULL;
bool TileDecoration::sCaching = false;

void TileDecoration::setupReusables(void)
{
	if (sCache)
        return;

	const int kNumKeys = 12;
    uint decorationKeys[kNumKeys] =
    {
        TilePiece::kTDKTeleport, TilePiece::kTDKColorSwap, TilePiece::kTDKRotate, TilePiece::kTDKShield,
        TilePiece::kTDKPainter, TilePiece::kTDKTileSwap, TilePiece::kTDKMirroredImage, TilePiece::kTDKKey,
        TilePiece::kTDKColorFill, TilePiece::kTDKPaintBomb, TilePiece::kTDKConveyorBelt, TilePiece::kTDKColorMagic
    };

    int cacheSizes[kNumKeys] =
    {
#if CM_SMALL_CACHES
    #ifdef CHEEKY_MOBILE
        60, 40, 30, 30,
        60, 30, 25, 16,
        30, 50, 25, 25
    #else
        60, 40, 30, 30,
        100, 30, 25, 16,
        30, 50, 25, 25
    #endif
#else
        75, 75, 40, 60,
        200, 40, 30, 25,
        50, 50, 50, 30
#endif
    };

    sCaching = true;
    sCache = new ReusableCache();

    for (int i = 0; i < kNumKeys; ++i)
    {
        int cacheSize = cacheSizes[i];
        uint reuseKey = decorationKeys[i];
        IReusable* reusable = NULL;
		sCache->addKey(cacheSize, reuseKey);

        for (int j = 0; j < cacheSize; ++j)
        {
            reusable = getTileDecoration(reuseKey, 0);
			reusable->hibernate();
			sCache->addReusable(reusable);
        }
    }

	sCache->verifyCacheIntegrity();
    sCaching = false;
}

IReusable* TileDecoration::checkoutReusable(uint reuseKey)
{
	IReusable* reusable = NULL;

    if (sCache && !sCaching)
		reusable = sCache->checkout(reuseKey);

    return reusable;
}

void TileDecoration::checkinReusable(IReusable* reusable)
{
	if (sCache && !sCaching && reusable)
		sCache->checkin(reusable);
}

TileDecoration* TileDecoration::getTileDecoration(uint type, uint subType)
{
	uint reuseKey = type;
    TileDecoration* tileDecoration = static_cast<TileDecoration*>(checkoutReusable(reuseKey));

    if (tileDecoration)
    {
        tileDecoration->reuse();
        tileDecoration->setSubType(subType);
    }
    else
    {
		tileDecoration = TileDecoration::create(type, subType, !sCaching);
    }
    
    return tileDecoration;
}

void TileDecoration::reuse(void)
{
	if (getInUse())
        return;

	setOpacityChildren(255);
    enableMenuMode(false);
    
    if (getType() == TilePiece::kTDKPainter)
        setScale(1.0f); // Menu can mess with us
	if (getType() == TilePiece::kTDKKey)
		setShaderProgram((mKeyShader ? mKeyShader : mNormalShader));
	if (isAdvanceable())
		mScene->addToJuggler(this);
    mInUse = true;
}

void TileDecoration::hibernate(void)
{
	if (!getInUse())
        return;
    
    enableUIMode(false);

	if (mTwinkle)
	{
		mTwinkle->stopAnimating();
		mTwinkle->setScale(0.0f);
		mTwinkle->setRotation(0);
		mTwinkle->setOpacity(0);
	}

	if (isAdvanceable())
		mScene->removeFromJuggler(this);

	setDecorator(NULL);
	removeFromParent();

    mInUse = false;
    checkinReusable(this);
}

void TileDecoration::returnToPool(void)
{
    if (getPoolIndex() != -1)
        hibernate();
    else
        removeFromParent();
}
