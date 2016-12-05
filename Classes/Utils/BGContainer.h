#ifndef __BG_CONTROLLER_H__
#define __BG_CONTROLLER_H__

#include <Interfaces/IEventListener.h>
#include "cocos2d.h"
USING_NS_CC;

class CroppedProp;
class PuzzleController;

class BGContainer : public IEventListener
{
public:
    BGContainer(void);
    ~BGContainer(void);
    
    void attachEventListeners(PuzzleController* puzzleController);
    void detachEventListeners(PuzzleController* puzzleController);
    
    void setViewableRegion(const CCRect& value);
    void setOcclusionRegion(const CCRect& value);
    void setBgCanvases(const std::vector<CroppedProp*>& canvases);
    
    void resetState(void);
    virtual void onEvent(int evType, void* evData);
    
private:
    void applyRegions(void);
    void setNumActiveExposures(int value);
    
    bool isAttachedToEvents;
    bool isPuzzleTransitioning;
    int numActiveExposures;
    CCRect viewableRegion;
    CCRect occlusionRegion;
    std::vector<CroppedProp*> bgCavases;
};
#endif // __TEXT_UTILS_H__
