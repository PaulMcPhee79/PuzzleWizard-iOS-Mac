#ifndef __ILICENSE_PROVIDER_H__
#define __ILICENSE_PROVIDER_H__

#include <string>

class IEventListener;

class ILicenseProvider
{
public:
	enum LicenseTest { APPROVED, DENIED, RETRY, ERROR, DIALOG_OK, DIALOG_RETRY, DIALOG_BUY };
	enum LicenseState { LS_PROVISIONAL, LS_APPROVED, LS_DENIED };

	ILicenseProvider(void) : mTest(APPROVED), mLicenseStatus(0), mSecsPlayed(0) { }
	virtual ~ILicenseProvider(void) { }

	virtual int getPollingCounterMax(void) = 0;
	virtual int getPollingCounterTimeout(void) = 0;
	virtual float getLicenseCheckInterval(void) = 0;
	virtual float getSecsPlayedBuyLimit(void) = 0;
	virtual float getLicenseCheckDelayPeriod(void) = 0;
	virtual std::string getGooglePlayLicenseKey(void) = 0;
	virtual int getGooglePlayLicenseStatus(void) = 0;
	virtual int getCachedGooglePlayLicenseStatus(void) = 0;
	virtual void setGooglePlayLicenseExpiration(int minsRemaining) = 0;
	virtual std::string googlePlayDialogTypeForSecsPlayed(float secsPlayed) = 0;
	virtual std::string googlePlayDialogTypeExpired(void) = 0;

	virtual bool verifyGooglePlayLicenseKey(const char* licenseKey) = 0;
	virtual void doGooglePlayLicenseCheck(void) = 0;
	virtual void showGooglePlayLicenseDialog(const char* dialogType) = 0;

	virtual void setTest(LicenseTest value) { mTest = value; }
	virtual void setGooglePlayLicenseStatus(int value) { mLicenseStatus = value; }
	virtual void setSecsPlayed(float value) { mSecsPlayed = value; }
	virtual void setDenied() = 0;
	virtual void setApproved() = 0;

	virtual void injectEvents(IEventListener* listener) { };
	virtual void extractEvents(IEventListener* listener) { };

	virtual void update(float dt) { }

protected:
	LicenseTest mTest;
	int mLicenseStatus;
	float mSecsPlayed;
};
#endif // __ILICENSE_PROVIDER_H__
