
#include "MenuSubview.h"
#include <SceneControllers/GameController.h>
USING_NS_CC;

MenuSubview::MenuSubview(void)
{
	setTag(GameController::GC()->getUnqiueKey());
}

MenuSubview::~MenuSubview(void)
{
	//CC_SAFE_RELEASE(xyz);
}
