#ifndef __LICENSE_TESTER_MANAGER_H__
#define __LICENSE_TESTER_MANAGER_H__

#include <Interfaces/ILicenseProvider.h>
#include <Events/EventDispatcher.h>

class LicenseManagerTester : public EventDispatcher, public ILicenseProvider
{
public:
	static int EV_TYPE_LICENSE_TEST_RESET_SECS_PLAYED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_LICENSE_TEST_DENIED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_LICENSE_TEST_APPROVED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	LicenseManagerTester(void);
	virtual ~LicenseManagerTester(void);

	virtual int getPollingCounterMax(void);
	virtual int getPollingCounterTimeout(void);
	virtual float getLicenseCheckInterval(void);
	virtual float getSecsPlayedBuyLimit(void);
	virtual float getLicenseCheckDelayPeriod(void);
	virtual std::string getGooglePlayLicenseKey(void);
	virtual int getGooglePlayLicenseStatus(void);
	virtual int getCachedGooglePlayLicenseStatus(void);
	virtual void setGooglePlayLicenseExpiration(int minsRemaining);
	virtual std::string googlePlayDialogTypeForSecsPlayed(float secsPlayed);
	virtual std::string googlePlayDialogTypeExpired(void);

	virtual bool verifyGooglePlayLicenseKey(const char* licenseKey);
	virtual void doGooglePlayLicenseCheck(void);
	virtual void showGooglePlayLicenseDialog(const char* dialogType);

	virtual void setDenied();
	virtual void setApproved();

	virtual void injectEvents(IEventListener* listener);
	virtual void extractEvents(IEventListener* listener);

	virtual void update(float dt);

private:
	int getGooglePlayDialogResponse(void);

	bool mIsTesting;
	bool mDidShowLicenseDialog;
};
#endif // __LICENSE_TESTER_MANAGER_H__
