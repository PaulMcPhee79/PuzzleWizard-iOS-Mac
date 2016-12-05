#ifndef __TF_MANAGER_H__
#define __TF_MANAGER_H__

#include "cocos2d.h"
USING_NS_CC;

class TFManager : public CCObject
{
public:
    enum TFState { NONE = 0, WELCOME_PENDING, WELCOME, THANKS_PENDING, THANKS };
    
	static TFManager* TFM(void);
    static void TFM_ASSERT_NULL(void);
	virtual ~TFManager(void);
    
    void applyPendingState(void);
    void clearPendingState(void);
    void update(float dt);
    
    void showWelcomeView(void);
    void showThanksView(void);
    
private:
	TFManager(void);
	TFManager(const TFManager& other);
	TFManager& operator=(const TFManager& rhs);
	void init(void);
    TFState getState(void) const { return mState; }
    void setState(TFState value);
    
    TFState mState;
};
#endif // __TF_MANAGER_H__
