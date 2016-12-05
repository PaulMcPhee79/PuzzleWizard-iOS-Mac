
#ifndef GOOGLE_PLAY_LICENSE_REQUIRED
//	#define GOOGLE_PLAY_LICENSE_REQUIRED
#endif

#include "LicenseManager.h"
#include "LicenseManagerTester.h"
#include <Utils/BridgingUtility.h>
#include <SceneControllers/SceneController.h>

static LicenseManager *g_sharedLicenseManager = NULL;

LicenseManager* LicenseManager::LM(void)
{
	if (g_sharedLicenseManager == NULL)
		g_sharedLicenseManager = new LicenseManager();
	return g_sharedLicenseManager;
}

LicenseManager::LicenseManager()
:
#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
mLicenseState(ILicenseProvider::LS_PROVISIONAL),
#else
mLicenseState(ILicenseProvider::LS_APPROVED),
#endif
mPollingCounter(0),
mPollingCounterTimeout(0),
mSecsPlayed(0),
mLicenseCheckIntervalTimer(0),
mLicenseProvider(new LicenseManagerTester())
{
	mLicenseProvider->injectEvents(this);

#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
	std::string licenseKey = SceneController::getUserDefaultString(CMSettings::S_LICENSE, "");
	if (!licenseKey.empty())
	{
		bool verified = mLicenseProvider->verifyGooglePlayLicenseKey(licenseKey.c_str());
		if (verified)
		{
			CCLog("License Key Verification Succeeded: %s", licenseKey.c_str());
			setApproved();
		}
		else
		{
			CCLog("License Key Verification Failed: %s", licenseKey.c_str());
			saveLicenseKey(""); // Clear the key - we could have restored from backup onto a new device
		}
	}
#else

#endif
}

// Under what reasonable circumstance would this ever be called?
LicenseManager::~LicenseManager(void)
{
	if (mLicenseProvider)
	{
		mLicenseProvider->extractEvents(this);
		delete mLicenseProvider, mLicenseProvider = NULL;
	}

	g_sharedLicenseManager = NULL; // Leave references valid
}

bool LicenseManager::hasDelayPeriodExpired(void) const
{
#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
	float delayPeriod = mLicenseProvider->getLicenseCheckDelayPeriod();
	return mSecsPlayed >= delayPeriod;
#else
	return false;
#endif
}

bool LicenseManager::hasBuyLimitExpired(void) const
{
#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
	float buyLimit = mLicenseProvider->getSecsPlayedBuyLimit();
	return mSecsPlayed >= buyLimit;
#else
	return false;
#endif
}

void LicenseManager::setDenied()
{
#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
	mLicenseState = ILicenseProvider::LS_DENIED;
	mLicenseProvider->setDenied();
	dispatchEvent(EV_TYPE_LICENSE_DENIED(), this);
#endif
}

void LicenseManager::setApproved()
{
#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
	mLicenseState = ILicenseProvider::LS_APPROVED;
	mLicenseProvider->setApproved();
	dispatchEvent(EV_TYPE_LICENSE_APPROVED(), this);
#endif
}

void LicenseManager::setSecsPlayed(float value)
{
	float prevValue = mSecsPlayed;
	mSecsPlayed = value;
	mLicenseProvider->setSecsPlayed(value);
}

void LicenseManager::setMinsPlayed(int value)
{
	float prevValue = mSecsPlayed;
	setSecsPlayed(value * 60);

	if (isLicenseApproved() == false)
	{
		float buyLimit = mLicenseProvider->getSecsPlayedBuyLimit();
		if (prevValue < buyLimit && mSecsPlayed >= buyLimit)
			dispatchEvent(EV_TYPE_LICENSE_TRIAL_EXPIRED(), this);
		else if (prevValue >= buyLimit && mSecsPlayed < buyLimit)
			dispatchEvent(EV_TYPE_LICENSE_TRIAL_EXTENDED(), this);
	}
}

void LicenseManager::setLicenseCheckHint(void)
{
	if (!isLicenseApproved() && !isLicenseDenied())
	{
		if (mSecsPlayed >= mLicenseProvider->getLicenseCheckDelayPeriod())
			mLicenseCheckIntervalTimer = 0;
	}
}

void LicenseManager::saveLicenseKey(const std::string& key)
{
#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
	SceneController::setUserDefault(CMSettings::S_LICENSE, key.c_str());
	SceneController::flushUserDefaults();
	CCLog("Saving GooglePlay license key:%s", key.c_str());
#endif
}

void LicenseManager::resetLicenseCheckIntervalTimer(void)
{
	mLicenseCheckIntervalTimer = mLicenseProvider->getLicenseCheckInterval();
}

void LicenseManager::resetPollingCounter(bool resetTimeout)
{
	if (resetTimeout)
		mPollingCounterTimeout = 0;
	mPollingCounter = mLicenseProvider->getPollingCounterMax();
	CCLog("LicenseManager::resetPollingCounter");
}

void LicenseManager::showLicenseDialog(void)
{
#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
	resetPollingCounter();

	if (isLicenseDenied())
	{
		mLicenseProvider->showGooglePlayLicenseDialog(mLicenseProvider->googlePlayDialogTypeExpired().c_str());
	}
	else
	{
		int expirationMinsRemaining = MAX(1, (int)((mLicenseProvider->getSecsPlayedBuyLimit() - mSecsPlayed) / 60));
		mLicenseProvider->setGooglePlayLicenseExpiration(expirationMinsRemaining);
		mLicenseProvider->showGooglePlayLicenseDialog(mLicenseProvider->googlePlayDialogTypeForSecsPlayed(mSecsPlayed).c_str());
	}

	CCLog("LicenseManager::showLicenseDialog");
#endif
}

void LicenseManager::doGooglePlayLicenseCheck(void)
{
#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
	resetPollingCounter();
	mLicenseProvider->doGooglePlayLicenseCheck();
	CCLog("LicenseManager::doGooglePlayLicenseCheck");
#endif
}

int LicenseManager::pollLicenseStatus(void)
{
#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
	int licenseStatus = mLicenseProvider->getGooglePlayLicenseStatus();
#else
	int licenseStatus = BridgingUtility::kGooglePlayLicenseApproved;
#endif

	if (licenseStatus == BridgingUtility::kGooglePlayLicenseInProgress && mPollingCounterTimeout >= mLicenseProvider->getPollingCounterTimeout())
		licenseStatus = BridgingUtility::kGooglePlayLicenseProvisionalRetry;

	switch (licenseStatus)
	{
		case BridgingUtility::kGooglePlayLicenseInProgress:
		{
			resetPollingCounter(false);
		}
			break;
		case BridgingUtility::kGooglePlayLicenseDenied:
		{
			setDenied();
			showLicenseDialog();
		}
			break;
		case BridgingUtility::kGooglePlayLicenseApproved:
		{
			setApproved();
			saveLicenseKey(mLicenseProvider->getGooglePlayLicenseKey());
		}
			break;
		case BridgingUtility::kGooglePlayLicenseProvisionalRetry:
		{
			if (hasDelayPeriodExpired())
			{
				if (hasBuyLimitExpired())
					dispatchEvent(EV_TYPE_LICENSE_TRIAL_EXPIRED(), this);
				showLicenseDialog();
			}
			else
				resetLicenseCheckIntervalTimer();
		}
			break;
		case BridgingUtility::kGooglePlayLicenseProvisionalError:
		{
			// This only happens when someone (Google or me) stuffed up. Don't punish the user for others' failure.
			setApproved();
			saveLicenseKey(mLicenseProvider->getGooglePlayLicenseKey());

//			float buyLimit = mLicenseProvider->getSecsPlayedBuyLimit();
//			if (mSecsPlayed >= buyLimit)
//				setSecsPlayed(buyLimit - mLicenseProvider->getLicenseCheckInterval());
//			resetLicenseCheckIntervalTimer();
		}
			break;
		case BridgingUtility::kGooglePlayLicenseDialogOK:
			resetLicenseCheckIntervalTimer();
			break;
		case BridgingUtility::kGooglePlayLicenseDialogRetry:
		{
			if (hasBuyLimitExpired())
				dispatchEvent(EV_TYPE_LICENSE_TRIAL_EXPIRED(), this);
			doGooglePlayLicenseCheck();
		}
			break;
		case BridgingUtility::kGooglePlayLicenseDialogBuy:
		{
			if (hasBuyLimitExpired())
				dispatchEvent(EV_TYPE_LICENSE_TRIAL_EXPIRED(), this);
			// In case 'Buy Now' was accidentally chosen, give the user another chance to choose 'Retry'
			showLicenseDialog();
		}
			break;
	}

	return licenseStatus;
}

void LicenseManager::onEvent(int evType, void* evData)
{
	if (evType == LicenseManagerTester::EV_TYPE_LICENSE_TEST_DENIED())
	{
		setDenied();
	}
	else if (evType == LicenseManagerTester::EV_TYPE_LICENSE_TEST_APPROVED())
	{
		setApproved();
		saveLicenseKey(mLicenseProvider->getGooglePlayLicenseKey());
	}
	else if (evType == LicenseManagerTester::EV_TYPE_LICENSE_TEST_RESET_SECS_PLAYED())
	{
		setMinsPlayed(0);
	}
}

void LicenseManager::update(float dt, float secsPlayedDt)
{
#ifdef GOOGLE_PLAY_LICENSE_REQUIRED
	mLicenseProvider->update(dt);

	if (isLicenseApproved() || isLicenseDenied())
	{
		int cachedLicenseStatus = mLicenseProvider->getCachedGooglePlayLicenseStatus();
		if (cachedLicenseStatus == BridgingUtility::kGooglePlayLicenseDenied || cachedLicenseStatus == BridgingUtility::kGooglePlayLicenseApproved)
			return;
	}
	else if (mSecsPlayed < mLicenseProvider->getSecsPlayedBuyLimit())
	{
		setSecsPlayed(mSecsPlayed + secsPlayedDt);

		if (mSecsPlayed < mLicenseProvider->getLicenseCheckDelayPeriod())
			return;
	}

	if (mPollingCounter > 0)
	{
		++mPollingCounterTimeout;
		if (--mPollingCounter == 0)
			pollLicenseStatus();
	}
	else if (mLicenseCheckIntervalTimer >= 0)
	{
		mLicenseCheckIntervalTimer -= dt;
		if (mLicenseCheckIntervalTimer < 0)
			doGooglePlayLicenseCheck();
	}
#endif
}
