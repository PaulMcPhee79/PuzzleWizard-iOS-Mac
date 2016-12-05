
#include "BGContainer.h"
#include <Prop/CroppedProp.h>
#include <Puzzle/View/Playfield/PuzzleBoard.h>
#include <Puzzle/Controllers/PuzzleController.h>

BGContainer::BGContainer(void)
    : isAttachedToEvents(false)
    , isPuzzleTransitioning(false)
    , numActiveExposures(0)
    , viewableRegion(CCRectZero)
    , occlusionRegion(CCRectZero)
{

}

BGContainer::~BGContainer(void)
{
    CCAssert(isAttachedToEvents == false, "BGContainer::~BGContainer should not be called while events are still attached.");
}

void BGContainer::attachEventListeners(PuzzleController* puzzleController)
{
    CCAssert(puzzleController, "BGContainer::attachEventListeners null arg.");
    puzzleController->addEventListener(PuzzleBoard::EV_TYPE_DID_TRANSITION_IN(), this);
    puzzleController->addEventListener(PuzzleBoard::EV_TYPE_BG_EXPOSURE_WILL_BEGIN(), this);
    puzzleController->addEventListener(PuzzleBoard::EV_TYPE_BG_EXPOSURE_DID_END(), this);
    isAttachedToEvents = true;
}

void BGContainer::detachEventListeners(PuzzleController* puzzleController)
{
    CCAssert(puzzleController, "BGContainer::detachEventListeners null arg.");
    puzzleController->removeEventListener(PuzzleBoard::EV_TYPE_DID_TRANSITION_IN(), this);
    puzzleController->removeEventListener(PuzzleBoard::EV_TYPE_BG_EXPOSURE_WILL_BEGIN(), this);
    puzzleController->removeEventListener(PuzzleBoard::EV_TYPE_BG_EXPOSURE_DID_END(), this);
    isAttachedToEvents = false;
}

void BGContainer::setViewableRegion(const CCRect& value)
{
    viewableRegion = value;
    applyRegions();
}

void BGContainer::setOcclusionRegion(const CCRect& value)
{
    occlusionRegion = value;
    applyRegions();
}

void BGContainer::applyRegions(void)
{
    CCAssert(bgCavases.size() == 5, "BGContainer::applyRegions requires exactly 5 canvases.");
    
    if (bgCavases.size() == 5)
    {
        // Center-piece (0)
        bgCavases[0]->setViewableRegion(occlusionRegion);
        bgCavases[0]->setVisible(numActiveExposures > 0);
        
        // Top/bottom pieces (1,2)
        bgCavases[1]->setViewableRegion(CCRectMake(
                                                   viewableRegion.getMinX(),
                                                   occlusionRegion.getMaxY(),
                                                   viewableRegion.size.width,
                                                   viewableRegion.getMaxY() - occlusionRegion.getMaxY()));
        bgCavases[2]->setViewableRegion(CCRectMake(
                                                   viewableRegion.getMinX(),
                                                   viewableRegion.getMinY(),
                                                   viewableRegion.size.width,
                                                   occlusionRegion.getMinY()));
        
        // Left/right pieces (3,4)
        bgCavases[3]->setViewableRegion(CCRectMake(
                                                   viewableRegion.getMinX(),
                                                   occlusionRegion.getMinY(),
                                                   occlusionRegion.getMinX(),
                                                   occlusionRegion.size.height));
        bgCavases[4]->setViewableRegion(CCRectMake(
                                                   occlusionRegion.getMaxX(),
                                                   occlusionRegion.getMinY(),
                                                   viewableRegion.getMaxX() - occlusionRegion.getMaxX(),
                                                   occlusionRegion.size.height));
    }
}

void BGContainer::setBgCanvases(const std::vector<CroppedProp*>& canvases)
{
    bgCavases.clear();
    bgCavases.insert(bgCavases.end(), canvases.begin(), canvases.end());
    applyRegions();
}

void BGContainer::setNumActiveExposures(int value)
{
    CCAssert(value >= 0, "BGContainer::setNumActiveExposures invalid arg.");
    numActiveExposures = MAX(0, value);
    
    if (bgCavases.size() > 0)
        bgCavases[0]->setVisible(numActiveExposures > 0);
}

void BGContainer::resetState(void)
{
    isPuzzleTransitioning = false;
    setNumActiveExposures(0);
}

void BGContainer::onEvent(int evType, void* evData)
{
    if (evType == PuzzleBoard::EV_TYPE_DID_TRANSITION_IN())
    {
        resetState();
    }
    else if (evType == PuzzleBoard::EV_TYPE_BG_EXPOSURE_WILL_BEGIN())
    {
        setNumActiveExposures(numActiveExposures + 1);
    }
    else if (evType == PuzzleBoard::EV_TYPE_BG_EXPOSURE_DID_END())
    {
        setNumActiveExposures(numActiveExposures - 1);
    }
}
