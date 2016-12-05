
#include "TFManager.h"
#include <Utils/BridgingUtility.h>
USING_NS_CC;

static TFManager *g_sharedTFManager = NULL;

TFManager* TFManager::TFM(void)
{
	if (!g_sharedTFManager)
	{
		g_sharedTFManager = new TFManager();
		g_sharedTFManager->init();
	}
	return g_sharedTFManager;
}

void TFManager::TFM_ASSERT_NULL(void)
{
    CCAssert(g_sharedTFManager == NULL, "TFManager must be NULL at this point to avoid OpenGL setup problems.");
}

TFManager::TFManager(void)
: mState(NONE)
{
    CCAssert(g_sharedTFManager == NULL, "Attempted to allocate a second instance of a singleton.");
}

TFManager::~TFManager(void)
{
    // Do nothing
}

void TFManager::init(void)
{
    // Do nothing
}

void TFManager::setState(TFState value)
{
    if (mState == value)
        return;
    
    // Clean up previous state
    TFState prevState = mState;
    switch (prevState)
    {
        case NONE:
            break;
        case WELCOME_PENDING:
            break;
        case WELCOME:
            break;
        case THANKS_PENDING:
            break;
        case THANKS:
            break;
        default:
            break;
    }
    
    mState = value;
    
    // Apply new state
    switch (mState)
    {
        case NONE:
            break;
        case WELCOME_PENDING:
            break;
        case WELCOME:
            CCAssert(prevState == WELCOME_PENDING, "Invalid state transition in TFManager.");
            BridgingUtility::showWelcomeView();
            break;
        case THANKS_PENDING:
            break;
        case THANKS:
            CCAssert(prevState == THANKS_PENDING, "Invalid state transition in TFManager.");
            BridgingUtility::showThanksView();
            break;
        default:
            break;
    }
}

void TFManager::applyPendingState(void)
{
    switch (mState)
    {
        case WELCOME_PENDING:
            setState(WELCOME);
            break;
        case THANKS_PENDING:
            setState(THANKS);
            break;
        default:
            break;
    }
}

void TFManager::clearPendingState(void)
{
    switch (mState)
    {
        case WELCOME_PENDING:
        case THANKS_PENDING:
            setState(NONE);
            break;
        default:
            break;
    }
    
}

void TFManager::update(float dt)
{
    switch (mState)
    {
        case WELCOME:
        case THANKS:
            // UIView has been hidden, so return to null state
            setState(NONE);
            break;
        default:
            break;
    }
}

void TFManager::showWelcomeView(void)
{
//    if (mState == NONE)
//        setState(WELCOME_PENDING);
}

void TFManager::showThanksView(void)
{
//    if (mState == NONE)
//        setState(THANKS_PENDING);
}
