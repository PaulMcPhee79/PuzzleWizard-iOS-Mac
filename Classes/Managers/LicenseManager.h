#ifndef __LICENSE_MANAGER_H__
#define __LICENSE_MANAGER_H__

#include "cocos2d.h"
#include <Events/EventDispatcher.h>
#include <Interfaces/ILicenseProvider.h>
USING_NS_CC;


class LicenseManager : public EventDispatcher, public IEventListener
{
public:
	static int EV_TYPE_LICENSE_DENIED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_LICENSE_APPROVED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_LICENSE_TRIAL_EXPIRED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_LICENSE_TRIAL_EXTENDED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	static LicenseManager* LM(void);
	virtual ~LicenseManager(void);
    
    bool isLicenseDenied(void) { return mLicenseState == ILicenseProvider::LS_DENIED; }
    bool isLicenseApproved(void) { return mLicenseState == ILicenseProvider::LS_APPROVED; }
    int getMinsPlayed(void) const { return (int)(mSecsPlayed / 60); }
    void setMinsPlayed(int value);
    void setLicenseCheckHint(void);

    void showLicenseDialog(void);
    virtual void onEvent(int evType, void* evData);
	void update(float dt, float secsPlayedDt);

private:
	LicenseManager(void);
	LicenseManager(const LicenseManager& other);
	LicenseManager& operator=(const LicenseManager& rhs);
	bool hasDelayPeriodExpired(void) const;
	bool hasBuyLimitExpired(void) const;
	void setDenied();
	void setApproved();
	void setSecsPlayed(float value);
	void saveLicenseKey(const std::string& key);
	void resetLicenseCheckIntervalTimer(void);
	void resetPollingCounter(bool resetTimeout = true);
	int pollLicenseStatus(void);
	void doGooglePlayLicenseCheck(void);

	ILicenseProvider::LicenseState mLicenseState;
    int mPollingCounter;
    int mPollingCounterTimeout;
    float mSecsPlayed;
    float mLicenseCheckIntervalTimer;
    ILicenseProvider* mLicenseProvider;
};
#endif // __LICENSE_MANAGER_H__
