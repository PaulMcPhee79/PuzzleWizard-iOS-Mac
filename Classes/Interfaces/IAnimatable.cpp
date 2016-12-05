
#include "IAnimatable.h"
#include <SceneControllers/SceneController.h>

SceneController* IAnimatable::s_Scene = NULL;

IAnimatable::~IAnimatable(void)
{
    if (mScene)
    {
        mScene->removeFromJuggler(this);
        mScene = NULL;
    }
}

SceneController* IAnimatable::getAnimatablesScene()
{
	return s_Scene;
}

void IAnimatable::setAnimatablesScene(SceneController* value)
{
	if (value)
		s_Scene = value;
}

void IAnimatable::relinquishAnimatablesScene(SceneController* scene)
{
	if (scene && scene == s_Scene)
		s_Scene = NULL;
}