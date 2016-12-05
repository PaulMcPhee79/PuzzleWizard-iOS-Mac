#ifndef __CMSPRITE_H__
#define __CMSPRITE_H__

#include "cocos2d.h"
#include <SceneControllers/SceneController.h>
#include <Interfaces/IAnimatable.h>
USING_NS_CC;

class CMSprite : public CCSprite, public IAnimatable
{
public:
	CMSprite(void);
	virtual ~CMSprite(void);

	static CMSprite* create(bool autorelease = true);
	static CMSprite* createWithSpriteFrame(CCSpriteFrame* pSpriteFrame, bool autorelease = true);
    static CMSprite* createWithTexture(CCTexture2D *pTexture, bool autorelease = true);

	virtual void advanceTime(float dt);

	bool isAdvanceable(void) const { return mAdvanceable; }
	void setAdvanceable(bool value) { mAdvanceable = value; }
	virtual void setOpacityChildren(GLubyte value);
    void setTextureCoordsCM(CCRect rect);

	static SceneController* getCMSpriteScene();
	static void setCMSpriteScene(SceneController* value);
	static void relinquishCMSpriteScene(SceneController* scene);

protected:
    bool mAdvanceable;
    SceneController* mScene;

private:
	static SceneController* s_Scene;
};
#endif // __CMSPRITE_H__
