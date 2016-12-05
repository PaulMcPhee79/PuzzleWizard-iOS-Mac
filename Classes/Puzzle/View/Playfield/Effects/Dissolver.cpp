
#include "Dissolver.h"
#include <Interfaces/IEventListener.h>
USING_NS_CC;

static const float kSpeedFactor = 1.33f;

Dissolver::Dissolver(int category, int dir, float minThreshold, float maxThreshold)
	:
	Prop(category),
	mRequestedDir(dir),
	mDir(dir ? dir : 1),
	mMinThreshold(minThreshold),
	mMaxThreshold(maxThreshold),
	mThreshold(dir == 1 ? minThreshold : maxThreshold),
	mBatch(NULL),
	mListener(NULL)
{

}

Dissolver::~Dissolver(void)
{
	CC_SAFE_RELEASE_NULL(mBatch);
	mListener = NULL;
}

Dissolver* Dissolver::createWith(int category, CCTexture2D* batchTex, int dir, float minThreshold, float maxThreshold, bool autorelease)
{
	Dissolver *dissolver = new Dissolver(category, dir, minThreshold, maxThreshold);
    if (dissolver && dissolver->initWith(batchTex))
    {
		if (autorelease)
			dissolver->autorelease();
        return dissolver;
    }
    CC_SAFE_DELETE(dissolver);
    return NULL;
}

bool Dissolver::initWith(CCTexture2D* batchTex, uint capacity)
{
	bool bRet = true;

	if (mBatch == NULL)
	{
		do
		{
			mBatch = new CCSpriteBatchNode();
			if (mBatch)
			{
				bRet = mBatch->initWithTexture(batchTex, 4 * capacity);
				if (bRet)
				{
					mBatch->setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey("Dissolve"));
					addChild(mBatch);
				}
			}
			else
				break;
		} while (false);
	}

	return bRet;
}

void Dissolver::addDissolvee(CCSprite* dissolvee)
{
	if (dissolvee && mBatch)
		mBatch->addChild(dissolvee);
}

void Dissolver::removeDissolvee(CCSprite* dissolvee)
{
	if (dissolvee && mBatch)
		mBatch->removeChild(dissolvee, true);
}

void Dissolver::clear()
{
	if (mBatch)
		mBatch->removeAllChildren();
}

void Dissolver::reset()
{
	mDir = mRequestedDir == 0 ? 1 : mRequestedDir;
	mThreshold = mDir == 1 ? mMinThreshold : mMaxThreshold;
}

void Dissolver::advanceTime(float dt)
{
	mThreshold += kSpeedFactor * mDir * dt;

	if (mDir > 0)
	{
		if (mThreshold > mMaxThreshold)
		{
			mThreshold = mMaxThreshold;
			if (mListener)
				mListener->onEvent(EV_TYPE_DISSOLVE_CYCLE_COMPLETED(), this);
			if (mRequestedDir == 0)
				mDir *= -1;
		}
	}
	else
	{
		if (mThreshold < mMinThreshold)
		{
			mThreshold = mMinThreshold;
			if (mListener)
				mListener->onEvent(EV_TYPE_DISSOLVE_CYCLE_COMPLETED(), this);
			if (mRequestedDir == 0)
				mDir *= -1;
		}
	}
}

void Dissolver::visit(void)
{
	if (mBatch)
	{
		CCGLProgram* glProgram = mBatch->getShaderProgram();
		glProgram->use();

		glUniform1f(mScene->uniformLocationByName("u_thresholdD"), mThreshold);
		GLuint texId = mScene->textureByName("noise")->getName();
		ccGLBindTexture2DN(mScene->textureUnitById(texId), texId);
	}

	Prop::visit();
}
