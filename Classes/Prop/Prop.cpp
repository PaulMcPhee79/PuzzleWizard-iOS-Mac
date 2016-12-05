
#include "Prop.h"
#include <Interfaces/IReusable.h>
#include <Utils/Utils.h>
USING_NS_CC;

SceneController* Prop::s_Scene = NULL;

Prop::Prop(int category)
:
mTryCache(false),
mAdvanceable(false),
mCategory(category),
mScene(s_Scene),
mOpacity(255)
{

}

Prop::~Prop(void)
{
	mScene = NULL;
}

Prop* Prop::createWithCategory(int category, bool autorelease)
{
    Prop *prop = new Prop(category);
	if (prop && prop->init())
    {
		if (autorelease)
			prop->autorelease();
        return prop;
    }
    CC_SAFE_DELETE(prop);
    return NULL;
}

int Prop::getCategory(void)
{
	return mCategory;
}

void Prop::setCategory(int value)
{
	mCategory = value;
}

void Prop::setOpacityChildren(GLubyte value)
{
	setOpacity(value);
	CMUtils::setOpacity(this, value);
}

void Prop::advanceTime(float dt) { }

void Prop::tryCache(void)
{
	IReusable* reusable = dynamic_cast<IReusable *>(this);
	if (reusable)
	{
		if (reusable->getPoolIndex() != -1)
			reusable->hibernate();
	}

	this->setTryCache(false);
}

SceneController* Prop::getPropsScene()
{
	return s_Scene;
}

void Prop::setPropsScene(SceneController* value)
{
	if (value)
		s_Scene = value;
}

void Prop::relinquishPropsScene(SceneController* scene)
{
	if (scene && scene == s_Scene)
		s_Scene = NULL;
}
