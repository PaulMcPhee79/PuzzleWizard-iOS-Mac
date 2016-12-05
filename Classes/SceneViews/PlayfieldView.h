#ifndef __PLAYFIELD_VIEW_H__
#define __PLAYFIELD_VIEW_H__

#include "cocos2d.h"
#include "SceneView.h"
#include <Interfaces/IInteractable.h>
#include <Interfaces/IResDependent.h>
USING_NS_CC;

class CMSprite;
class CroppedProp;
class PlayfieldController;
class BGContainer;

class PlayfieldView : public SceneView, public IInteractable, public IResDependent
{
public:
	PlayfieldView(PlayfieldController* controller);
	virtual ~PlayfieldView(void);

	virtual void setupView();
	virtual void attachEventListeners();
	virtual void detachEventListeners();
	void enableMenuMode(bool enable);
    void enableLowPowerMode(bool enable);
    virtual void resolutionDidChange(void);
	virtual void onEvent(int evType, void* evData);
	virtual void advanceTime(float dt);
    
    virtual unsigned int getInputFocus(void) const { return CMInputs::HAS_FOCUS_BOARD; }
    virtual void update(int controllerState);

private:
    CCRect getBgOcclusionRegion(void);
    
	float mElapsedTime;
    float mTotalElapsedTime;
	float mGradientX;
	float mThreshold;
	int mThresholdDir;
    
	CMSprite* mBgImage;
    Prop* mBgCanvas;
    
    BGContainer* mBgContainer;
    std::vector<CMSprite*> mBgImages;
    std::vector<CroppedProp*> mBgCavases;
	
	PlayfieldController* mController;
};
#endif // __PLAYFIELD_VIEW_H__
