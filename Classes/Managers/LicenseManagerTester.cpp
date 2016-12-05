
#ifndef GOOGLE_PLAY_LICENSE_REQUIRED
//	#define GOOGLE_PLAY_LICENSE_REQUIRED
#endif

#include "LicenseManagerTester.h"
#include <Utils/BridgingUtility.h>
#include "cocos2d.h"
USING_NS_CC;

#if CM_BETA

static const float kLicenseCheckDelayPeriod = 30 * 60;	// Refund window is ~15 mins, before which license checks will return LICENSED
static const float kSecsPlayedBuyLimit = 90 * 60 + 50;	// Add 50 secs so that most users sees round values (15min, 30min remaining etc) before it goes out-of-sync
static const float kLicenseCheckInterval = 15 * 60;
static const int kPollingCounterMax = 30;
static const int kPollingCounterTimeout = 30 * kPollingCounterMax;

#else
// Add 30 secs so that user sees round values (15min, 30min etc)
static const float kLicenseCheckDelayPeriod = 30 * 60; 	// Refund window is ~15 mins, before which license checks will return LICENSED
static const float kSecsPlayedBuyLimit = 150 * 60 + 50;	// Add 50 secs so that most users sees round values (15min, 30min remaining etc) before it goes out-of-sync
static const float kLicenseCheckInterval = 15 * 60;
static const int kPollingCounterMax = 30;
static const int kPollingCounterTimeout = 30 * kPollingCounterMax;

#endif

LicenseManagerTester::LicenseManagerTester()
:
mIsTesting(false),
mDidShowLicenseDialog(false)
{
	mTest = APPROVED;
	mLicenseStatus = BridgingUtility::kGooglePlayLicenseProvisionalRetry;
}

LicenseManagerTester::~LicenseManagerTester(void)
{
	// Do nothing
}

void LicenseManagerTester::injectEvents(IEventListener* listener)
{
	if (listener)
	{
		addEventListener(LicenseManagerTester::EV_TYPE_LICENSE_TEST_DENIED(), listener);
		addEventListener(LicenseManagerTester::EV_TYPE_LICENSE_TEST_APPROVED(), listener);
		addEventListener(LicenseManagerTester::EV_TYPE_LICENSE_TEST_RESET_SECS_PLAYED(), listener);
	}
}

void LicenseManagerTester::extractEvents(IEventListener* listener)
{
	if (listener)
	{
		removeEventListener(LicenseManagerTester::EV_TYPE_LICENSE_TEST_DENIED(), listener);
		removeEventListener(LicenseManagerTester::EV_TYPE_LICENSE_TEST_APPROVED(), listener);
		removeEventListener(LicenseManagerTester::EV_TYPE_LICENSE_TEST_RESET_SECS_PLAYED(), listener);
	}
}

void LicenseManagerTester::setDenied()
{
	mLicenseStatus = BridgingUtility::kGooglePlayLicenseDenied;
}

void LicenseManagerTester::setApproved()
{
	mLicenseStatus = BridgingUtility::kGooglePlayLicenseApproved;
}

int LicenseManagerTester::getGooglePlayDialogResponse(void)
{
	if (mDidShowLicenseDialog)
	{
		mDidShowLicenseDialog = false;
		return BridgingUtility::getGooglePlayLicenseStatus();
	}
	else
		return mLicenseStatus;
}

int LicenseManagerTester::getPollingCounterMax(void)
{
	return mIsTesting ? kPollingCounterMax : kPollingCounterMax;
}

int LicenseManagerTester::getPollingCounterTimeout(void)
{
	return kPollingCounterTimeout;
}

float LicenseManagerTester::getLicenseCheckInterval(void)
{
	return mIsTesting ? 15.0f : kLicenseCheckInterval;
}

float LicenseManagerTester::getSecsPlayedBuyLimit(void)
{
	return mIsTesting ? 60.0f : kSecsPlayedBuyLimit;
}

float LicenseManagerTester::getLicenseCheckDelayPeriod(void)
{
	return mIsTesting ? 10.0f : kLicenseCheckDelayPeriod;
}

void LicenseManagerTester::setGooglePlayLicenseExpiration(int minsRemaining)
{
	BridgingUtility::setGooglePlayLicenseExpiration(minsRemaining);
}

std::string LicenseManagerTester::googlePlayDialogTypeForSecsPlayed(float secsPlayed)
{
	bool okDialog = secsPlayed < getSecsPlayedBuyLimit();
	if (okDialog)
		return "OK_RETRY";
	else
		return googlePlayDialogTypeExpired();
}

std::string LicenseManagerTester::googlePlayDialogTypeExpired(void)
{
	return "BUY_RETRY";
}

std::string LicenseManagerTester::getGooglePlayLicenseKey(void)
{
	return BridgingUtility::getGooglePlayLicenseKey();
}

bool LicenseManagerTester::verifyGooglePlayLicenseKey(const char* licenseKey)
{
	if (licenseKey && mIsTesting == false)
		return BridgingUtility::verifyGooglePlayLicenseKey(licenseKey);
	else
		return false;
}

void LicenseManagerTester::doGooglePlayLicenseCheck(void)
{
	mLicenseStatus = BridgingUtility::kGooglePlayLicenseInProgress;

	if (!mIsTesting)
		BridgingUtility::doGooglePlayLicenseCheck();
}

void LicenseManagerTester::showGooglePlayLicenseDialog(const char* dialogType)
{
	mDidShowLicenseDialog = true;
	mLicenseStatus = BridgingUtility::kGooglePlayLicenseInProgress;
	BridgingUtility::showGooglePlayLicenseDialog(dialogType);
}

int LicenseManagerTester::getGooglePlayLicenseStatus(void)
{
#ifndef GOOGLE_PLAY_LICENSE_REQUIRED
	return BridgingUtility::kGooglePlayLicenseApproved;
#else
	if (mIsTesting)
	{
		switch (mTest)
		{
			case APPROVED:
			{
				if (mLicenseStatus == BridgingUtility::kGooglePlayLicenseInProgress)
				{
					mLicenseStatus = BridgingUtility::kGooglePlayLicenseApproved;
				}
			}
				break;
			case DENIED:
			{
				if (mLicenseStatus == BridgingUtility::kGooglePlayLicenseInProgress)
				{
					mLicenseStatus = BridgingUtility::kGooglePlayLicenseDenied;

					if (getGooglePlayDialogResponse() == BridgingUtility::kGooglePlayLicenseDialogRetry)
					{
						CCLog("LicenseManagerTester::DENIED_DIALOG_RETRY");
						return BridgingUtility::kGooglePlayLicenseDialogRetry;
					}
					else
						CCLog("LicenseManagerTester::DENIED");
				}
			}
				break;
			case RETRY:
			case ERROR:
			{
				if (mLicenseStatus == BridgingUtility::kGooglePlayLicenseInProgress)
				{
					mLicenseStatus = mTest == RETRY
							? BridgingUtility::kGooglePlayLicenseProvisionalRetry
							: BridgingUtility::kGooglePlayLicenseProvisionalError;

					int dialogResponse = getGooglePlayDialogResponse();
					if (dialogResponse != mLicenseStatus)
						return dialogResponse;
				}
			}
				break;
			case DIALOG_RETRY:
			{
				if (mLicenseStatus == BridgingUtility::kGooglePlayLicenseInProgress)
				{
					static int queryCounter = 0;
					if (++queryCounter < 4) // 2 dialogs worth
					{
						mLicenseStatus = BridgingUtility::kGooglePlayLicenseProvisionalRetry;
						int dialogResponse = getGooglePlayDialogResponse();
						if (dialogResponse != mLicenseStatus)
							return dialogResponse;
					}
					else
						mLicenseStatus = BridgingUtility::kGooglePlayLicenseDenied; //BridgingUtility::kGooglePlayLicenseApproved;
				}
			}
				break;
			case DIALOG_OK:
			case DIALOG_BUY:
			{
				if (mLicenseStatus == BridgingUtility::kGooglePlayLicenseInProgress)
				{
					mLicenseStatus = BridgingUtility::kGooglePlayLicenseProvisionalRetry;
					int dialogResponse = getGooglePlayDialogResponse();
					if (dialogResponse != mLicenseStatus)
						return dialogResponse;
				}
			}
				break;
			default:
				break;
		}
	}
	else
		mLicenseStatus = BridgingUtility::getGooglePlayLicenseStatus();

	return mLicenseStatus;
#endif
}

int LicenseManagerTester::getCachedGooglePlayLicenseStatus(void)
{
	return mLicenseStatus;
}

void LicenseManagerTester::update(float dt)
{
	static bool hasResetSecsPlayed = false;
	if (mIsTesting && hasResetSecsPlayed == false)
	{
		// Cancel out saved time passed so we can conduct the test from the origin
		hasResetSecsPlayed = true;
		dispatchEvent(EV_TYPE_LICENSE_TEST_RESET_SECS_PLAYED(), this);
	}
}
