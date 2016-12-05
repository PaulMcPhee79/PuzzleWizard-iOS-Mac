
#include "PlayfieldView.h"
#include "AppMacros.h"
#include <Utils/Utils.h>
#include <Utils/Globals.h>
#include <Extensions/MovieClip.h>
#include <Puzzle/View/Playfield/TileDecoration.h>
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Puzzle/View/Playfield/AnimPlayerPiece.h>
#include <Puzzle/Data/HumanPlayer.h>
#include <Puzzle/View/PuzzleHelper.h>
#include <Puzzle/Data/Puzzle.h>
#include <Puzzle/Controllers/PuzzleController.h>
#include <Puzzle/View/Playfield/PuzzleBoard.h>
#include <Managers/ControlsManager.h>
#include <Prop/CMSprite.h>
#include <SceneControllers/PlayfieldController.h>
#include <Prop/CroppedProp.h>
#include <Utils/DeviceDetails.h>
#include <Utils/BgContainer.h>
#include <math.h>
USING_NS_CC;

PlayfieldView::PlayfieldView(PlayfieldController* controller)
    : mBgImage(NULL)
    , mBgCanvas(NULL)
    , mBgContainer(NULL)
{
	mController = controller;
	mElapsedTime = mTotalElapsedTime = mGradientX = mThreshold = 0;
	mThresholdDir = 1;
}

PlayfieldView::~PlayfieldView(void)
{
    if (mController)
    {
        mController->deregisterResDependent(this);
        mController->unsubscribeToInputUpdates(this);
        mController = NULL;
    }
    
//	CC_SAFE_RELEASE_NULL(mBgImage);
//	CC_SAFE_RELEASE_NULL(mBgCanvas);
    
    for (std::vector<CMSprite*>::iterator it = mBgImages.begin(); it != mBgImages.end(); ++it)
        CC_SAFE_RELEASE(*it);
    mBgImages.clear();

    for (std::vector<CroppedProp*>::iterator it = mBgCavases.begin(); it != mBgCavases.end(); ++it)
        CC_SAFE_RELEASE(*it);
    mBgCavases.clear();
    
    if (mBgContainer)
        mBgContainer->detachEventListeners(mController->getPuzzleController());
    delete mBgContainer, mBgContainer = NULL;
}

void PlayfieldView::setupView()
{
	//SceneView::setupView(); // abstract

//	mBgCanvas = Prop::createWithCategory(CMGlobals::BG, false);
//	mController->addProp(mBgCanvas);
//
//	//mFgCanvas = Prop::createWithCategory(CMGlobals::ELEVATED_EFFECTS, false);
//	//mController->addProp(mFgCanvas);
//
//	// Refraction
//	mBgImage = new CMSprite();
//	mBgImage->initWithTexture(mController->textureByName("bg"));
//	mBgImage->setScaleX(mController->getInvViewScale().x * mController->getViewportWidth() / mBgImage->boundingBox().size.width);
//	mBgImage->setScaleY(mController->getInvViewScale().y * mController->getViewportHeight() / mBgImage->boundingBox().size.height);
//	// Uncomment to maintain aspect ratio (more expensive to draw).
//	//mBgImage->setScale(mController->getMaximizingContentScaleFactor() * mController->getViewWidth() / mBgImage->boundingBox().size.width);
//	mBgImage->setPosition(ccp(mController->getViewWidth() / 2, mController->getViewHeight() / 2));
//	mBgCanvas->addChild(mBgImage);
    
    
    
    // New BG code
    CCRect bgViewableRegion = CCRectMake(0, 0, mController->getViewWidth(), mController->getViewHeight());
    {
        ccTexParams params = {GL_LINEAR,GL_LINEAR,GL_REPEAT,GL_REPEAT};
        
        for (int i = 0; i < 5; ++i)
        {
            CMSprite* bgImage = new CMSprite();
            bgImage->initWithTexture(mController->textureByName("bg"));
            bgImage->getTexture()->setTexParameters(&params);
            bgImage->setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey("Refraction"));
            bgImage->setScaleX(mController->getInvViewScale().x * (mController->getViewportWidth() / bgImage->boundingBox().size.width));
            bgImage->setScaleY(mController->getInvViewScale().y * (mController->getViewportHeight() / bgImage->boundingBox().size.height));
            bgImage->setPosition(ccp(mController->getViewWidth() / 2, mController->getViewHeight() / 2));
            mBgImages.push_back(bgImage);
            
            CroppedProp* bgCanvas = new CroppedProp(CMGlobals::BG, bgViewableRegion);
            bgCanvas->addChild(bgImage);
            mController->addProp(bgCanvas);
            mBgCavases.push_back(bgCanvas);
        }
    }
    
    mBgContainer = new BGContainer();
    mBgContainer->setBgCanvases(mBgCavases);
    mBgContainer->setViewableRegion(CCRectMake(
                                               0,
                                               0,
                                               ceil(mController->getViewportWidth()),
                                               ceil(mController->getViewportHeight())));
    
    mBgContainer->attachEventListeners(mController->getPuzzleController());

	/*CCSprite* temp = CMUtils::createColoredQuad(CCSizeMake(1024, 768));
	temp->setColor(CMUtils::uint2color3B(0xe800f1));
	temp->setPosition(ccp(mController->getViewWidth() / 2, mController->getViewHeight() / 2));
	mBgCanvas->addChild(temp);*/

    //if (CM_getPlatformType() != UIDevice1GiPad)
//    {
//        ccTexParams params = {GL_LINEAR,GL_LINEAR,GL_REPEAT,GL_REPEAT};
//        mBgImage->getTexture()->setTexParameters(&params);
//        mBgImage->setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey("Refraction"));
//    }

	PuzzleBoard* puzzleBoard = mController->getPuzzleController()->getPuzzleBoard();
	mController->addProp(puzzleBoard);

	vec2 tileDims = puzzleBoard->getScaledTileDimensions(), boardDims = puzzleBoard->getScaledBoardDimensions();
	mController->getPuzzleController()->setPuzzleBoardPosition(ccp(
		(mController->getViewWidth() - boardDims.x) / 2 + tileDims.x / 2,
		//(mController->getViewHeight() - boardDims.y) / 2 + tileDims.y / 2));
        (mController->getViewHeight() - boardDims.y) / 2 + tileDims.y / 2));
    
    CCRect occlusionRegion = getBgOcclusionRegion();
    mBgContainer->setOcclusionRegion(occlusionRegion);

    mController->registerResDependent(this);
    mController->subscribeToInputUpdates(this);
}

CCRect PlayfieldView::getBgOcclusionRegion(void)
{
    CCRect occlusionRegion = mController->getPuzzleController()->getPuzzleBoard()->getGlobalBoardClippingBounds();
    occlusionRegion.setRect(ceil(occlusionRegion.origin.x + 2),
                            ceil(occlusionRegion.origin.y),
                            floor(occlusionRegion.size.width - 4),
                            floor(occlusionRegion.size.height - 4));
    return occlusionRegion;
}

void PlayfieldView::attachEventListeners()
{
	
}

void PlayfieldView::detachEventListeners()
{

}

void PlayfieldView::enableMenuMode(bool enable)
{
    if (mBgImage)
        mBgImage->setVisible(!enable);
    for (std::vector<CroppedProp*>::iterator it = mBgCavases.begin(); it != mBgCavases.end(); ++it)
        (*it)->setVisible(!enable);
}

void PlayfieldView::enableLowPowerMode(bool enable)
{
    // TODO
}

void PlayfieldView::resolutionDidChange(void)
{
//    if (mBgImage)
//    {
//        mBgImage->setScale(1.0f);
//        mBgImage->setScaleX(mController->getInvViewScale().x * mController->getViewportWidth() / mBgImage->boundingBox().size.width);
//        mBgImage->setScaleY(mController->getInvViewScale().y * mController->getViewportHeight() / mBgImage->boundingBox().size.height);
//        // Uncomment to maintain aspect ratio (more expensive to draw).
//        //mBgImage->setScale(mController->getMaximizingContentScaleFactor() * mController->getViewWidth() / mBgImage->boundingBox().size.width);
//        mBgImage->setPosition(ccp(mController->getViewWidth() / 2, mController->getViewHeight() / 2));
//    }
    
    for (std::vector<CMSprite*>::iterator it = mBgImages.begin(); it != mBgImages.end(); ++it)
    {
        CMSprite* bgImage = *it;
        bgImage->setScale(1.0f);
        bgImage->setScaleX(mController->getInvViewScale().x * mController->getViewportWidth() / bgImage->boundingBox().size.width);
        bgImage->setScaleY(mController->getInvViewScale().y * mController->getViewportHeight() / bgImage->boundingBox().size.height);
        // Uncomment to maintain aspect ratio (more expensive to draw).
        //mBgImage->setScale(mController->getMaximizingContentScaleFactor() * mController->getViewWidth() / mBgImage->boundingBox().size.width);
        bgImage->setPosition(ccp(mController->getViewWidth() / 2, mController->getViewHeight() / 2));
    }
    
    if (mBgContainer)
    {
        mBgContainer->setViewableRegion(CCRectMake(
                                                   0,
                                                   0,
                                                   ceil(mController->getViewportWidth()),
                                                   ceil(mController->getViewportHeight())));
        
        CCRect occlusionRegion = getBgOcclusionRegion();
        mBgContainer->setOcclusionRegion(occlusionRegion);
    }
}

void PlayfieldView::onEvent(int evType, void* evData)
{

}

void PlayfieldView::update(int controllerState)
{
#ifndef __ANDROID__
    if (ControlsManager::CM()->didKeyDepress(kVK_Escape))
        mController->showMenuDialog("PlayfieldEsc");
#endif
}

const float PIx20 = 62.83185307f * 1.5f;
void PlayfieldView::advanceTime(float dt)
{
    //if (CM_getPlatformType() != UIDevice1GiPad)
    {
        mElapsedTime += dt;
        mTotalElapsedTime += dt;
        
        if (mElapsedTime > PIx20)
            mElapsedTime -= PIx20;
        
        CCGLProgram *glProgram = mBgImages[0]->getShaderProgram(); //mBgImage->getShaderProgram();
        glProgram->use();

        CCPoint displacementScroll = CMUtils::moveInCircle(mTotalElapsedTime, 0.05f);
		// Shader doesn't like modding large numbers (or if it does, it loses accuracy)
        glUniform1f(mController->uniformLocationByName("u_scrollR"), -mElapsedTime / PIx20);
        glUniform2f(mController->uniformLocationByName("u_displacementScrollR"), displacementScroll.x, displacementScroll.y);
       // glUniform4f(mController->uniformLocationByName("u_displacementFactorR"), 0.2f, 0.1875f, 0.2f, 0.175f);
        GLuint texId = mController->textureByName("refraction")->getName();
        ccGLBindTexture2DN(mController->textureUnitById(texId), texId);
    }
}
