#ifndef __PROP_H__
#define __PROP_H__

#include "cocos2d.h"
#include <SceneControllers/SceneController.h>
#include <Interfaces/IAnimatable.h>
USING_NS_CC;

class Prop : public CCNode, public IAnimatable
{
public:
	Prop(int category = -1);
	virtual ~Prop(void);

	static Prop* createWithCategory(int category, bool autorelease = true);
	virtual bool init(void) { return CCNode::init(); }

	int getCategory(void);
	void setCategory(int value);
	bool isAdvanceable(void) { return mAdvanceable; }

	GLubyte getOpacity(void) const { return mOpacity; }
	virtual void setOpacity(GLubyte value) { mOpacity = value; }
	virtual void setOpacityChildren(GLubyte value);

	virtual void advanceTime(float dt);

	bool getTryCache(void) { return mTryCache; }
	void setTryCache(bool value) { mTryCache = value; }
	virtual void tryCache(void);

	static SceneController* getPropsScene();
	static void setPropsScene(SceneController* value);
	static void relinquishPropsScene(SceneController* scene);

protected:
	int mCategory;
    bool mAdvanceable;
    SceneController* mScene;

private:
	bool mTryCache;
	GLubyte mOpacity;
	static SceneController* s_Scene;
};
#endif // __PROP_H__
