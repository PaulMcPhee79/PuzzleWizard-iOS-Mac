#ifndef __SCENE_VIEW_H__
#define __SCENE_VIEW_H__

#include "cocos2d.h"
#include <SceneControllers/SceneController.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
USING_NS_CC;

class SceneView : public CCObject, public EventDispatcher, public IEventListener
{
public:
	SceneView(void);
	virtual ~SceneView(void);

	virtual void setupView() = 0;
	virtual void attachEventListeners() { }
	virtual void detachEventListeners() { }
	virtual void onEvent(int evType, void* evData) { }
	virtual void advanceTime(float dt) { }
};
#endif // __SCENE_VIEW_H__
