
#define FLURRY_ENABLED 0
#define RATE_THE_GAME_TESTING 0

#include "BridgingUtility.h"
#include <SceneControllers/SceneController.h>
#include <SceneControllers/GameController.h>
#include <Managers/GameSettings.h>
#include <Puzzle/Controllers/GameProgressController.h>
#include "SimpleAudioEngine.h"
#include <CMJniUtils.h>

static const size_t kEngineLockLenMax = 64;
static char g_engineLock[kEngineLockLenMax+1] = "";

bool BridgingUtility::isFullscreen(void)
{
    return false;
}

void BridgingUtility::setFullscreen(bool value)
{

}

bool BridgingUtility::isIOSFeatureSupported(const char* reqSysVer)
{
	return false;
}

static const int kMinSolvedQuotaBeforePrompting = 12;
bool BridgingUtility::shouldPromptForRating(void)
{
#if CM_BETA
    return false;
#elif DEBUG && RATE_THE_GAME_TESTING
    return true;
#else
    int solvedCountAtLastPrompt = SceneController::getUserDefaultInt(CMSettings::I_RATE_GAME, 0);
    int solvedCountThisPrompt = GameProgressController::GPC()->getNumSolvedPuzzles();

    if (solvedCountAtLastPrompt == -1)
        return false;
    else if (solvedCountThisPrompt >= kMinSolvedQuotaBeforePrompting)
        return true;
    else
        return false;

//	else if (solvedCountThisPrompt > 36)
//		return false; // Stop bothering them at this point
//	else if (solvedCountThisPrompt >= kMinSolvedQuotaBeforePrompting && solvedCountThisPrompt - solvedCountAtLastPrompt >= kSolvedQuotaBetweenPrompts)
//		return true;
//	else
//		return false;
#endif
}

void BridgingUtility::userRespondedToRatingPrompt(bool response)
{
	if (response)
	{
		SceneController::setUserDefault(CMSettings::I_RATE_GAME, -1); // -1 means don't ask anymore because they agreed to rate it already.
		bool didRate = true;
		GameController::GC()->notifyEvent(GameController::EV_TYPE_USER_DID_RATE_THE_GAME(), (void*)&didRate);
		CMJniUtils::openMarketPlaceToRateTheGame();
	}
	else
	{
		SceneController::setUserDefault(CMSettings::I_RATE_GAME, GameProgressController::GPC()->getNumSolvedPuzzles());
	}

	SceneController::flushUserDefaults();
}

void BridgingUtility::userDidRateTheGame(bool success)
{
	SceneController::setUserDefault(CMSettings::I_RATE_GAME, success ? -1 : 0);
	SceneController::flushUserDefaults();
}

void BridgingUtility::pauseEngine(void)
{
#ifdef CHEEKY_MOBILE
    CCDirector::sharedDirector()->stopAnimation();
    CCDirector::sharedDirector()->pause();
#endif
}

bool BridgingUtility::pauseEngine(const char* lock)
{
    if (lock && strlen(lock) <= kEngineLockLenMax && *g_engineLock == '\0')
    {
    	strcpy(g_engineLock, lock);
        BridgingUtility::pauseEngine();
        return true;
    } else
        return false;
}

void BridgingUtility::resumeEngine(void)
{
    if (*g_engineLock == '\0')
    {
        CCDirector::sharedDirector()->resume();
        CCDirector::sharedDirector()->startAnimation();
    }
}

bool BridgingUtility::resumeEngine(const char* unlock)
{
    if (*g_engineLock && unlock && strcmp(g_engineLock, unlock) == 0)
    {
    	*g_engineLock = '\0';
        BridgingUtility::resumeEngine();
        return true;
    } else
        return false;
}

std::vector<std::string> BridgingUtility::getPreferredLanguageCodes(void)
{
	// Not really used on Android
	return std::vector<std::string>(1, "en");
}

bool BridgingUtility::isHighPerformanceMachine(void)
{
	return false;
}

bool BridgingUtility::isMainThread(void)
{
	return true;
}

std::string BridgingUtility::getSplashImagePath(void)
{
	return "";
}

bool BridgingUtility::isPad(void)
{
	return false;
}

void BridgingUtility::postNotification(const char* name)
{

}

void BridgingUtility::openURL(const char* url)
{
	CMJniUtils::openURL(url);
}

void BridgingUtility::openFileURL(const char* url) { }

void BridgingUtility::setIdleTimerDisabled(bool disabled) { }

void BridgingUtility::setStatusBarHidden(bool hidden) { }

void BridgingUtility::enableStatusBarLightColorStyle(bool enable) { }

std::string BridgingUtility::localizedString(const char* str)
{
	return CMJniUtils::getResStringForKey(str);
}

int BridgingUtility::alertButtonReturnToZeroBasedIndex(int value)
{
	return value;
}

void BridgingUtility::showAlertView(IEventListener* listener, int tag, const std::vector<std::string>& config, bool cancelButton) { }


void BridgingUtility::doGooglePlayLicenseCheck(void)
{
	CMJniUtils::doGooglePlayLicenseCheck();
}

int BridgingUtility::getGooglePlayLicenseStatus(void)
{
	return CMJniUtils::getGooglePlayLicenseStatus();
}

void BridgingUtility::setGooglePlayLicenseExpiration(int minsRemaining)
{
	CMJniUtils::setGooglePlayLicenseExpiration(minsRemaining);
}

bool BridgingUtility::verifyGooglePlayLicenseKey(const char* licenseKey)
{
	return CMJniUtils::verifyGooglePlayLicenseKey(licenseKey);
}

std::string BridgingUtility::getGooglePlayLicenseKey(void)
{
	return CMJniUtils::getGooglePlayLicenseKey();
}

void BridgingUtility::showGooglePlayLicenseDialog(const char* dialogType)
{
	CMJniUtils::showGooglePlayLicenseDialog(dialogType);
}

// Common public interface
const char* BridgingUtility::CRASH_CONTEXT_GAME_STATE = "GAME_STATE";
const char* BridgingUtility::CRASH_CONTEXT_LEVEL_NAME = "LEVEL_NAME";
const char* BridgingUtility::CRASH_CONTEXT_PUZZLE_NAME = "PUZZLE_NAME";
const char* BridgingUtility::CRASH_CONTEXT_LOCALE = "LOCALE";
const char* BridgingUtility::CRASH_CONTEXT_MENU = "MENU";
const char* BridgingUtility::CRASH_CONTEXT_SFX = "SFX";
const char* BridgingUtility::CRASH_CONTEXT_MUSIC = "MUSIC";
const char* BridgingUtility::CRASH_CONTEXT_ICLOUD = "ICLOUD";
const char* BridgingUtility::CRASH_CONTEXT_COLOR_BLIND = "COLOR_BLIND";
const char* BridgingUtility::CONTEXT_ENABLED = "Enabled";
const char* BridgingUtility::CONTEXT_DISABLED = "Disabled";

const char* BridgingUtility::EV_NAME_LOCALE_CHANGED = "LOCALE_CHANGED";
const char* BridgingUtility::EV_NAME_LEVEL_UNLOCKED = "LEVEL_UNLOCKED";
const char* BridgingUtility::EV_NAME_LEVEL_COMPLETED = "LEVEL_COMPLETED";
const char* BridgingUtility::EV_NAME_PUZZLE_ATTEMPTED = "PUZZLE_ATTEMPTED";
const char* BridgingUtility::EV_NAME_PUZZLE_SOLVED = "PUZZLE_SOLVED";
const char* BridgingUtility::EV_NAME_RATED_THE_GAME = "RATED_THE_GAME";
const char* BridgingUtility::EV_NAME_BETA_PUZZLE_BUGGED = "PUZZLE_BUGGED";
const char* BridgingUtility::EV_NAME_BETA_SUGGEST_IQ_ = "SUGGEST_IQ_";

const char* BridgingUtility::bool2Context(bool value)
{
    return value ? CONTEXT_ENABLED : CONTEXT_DISABLED;
}

void BridgingUtility::startSession(bool reportCrashes) { }
void BridgingUtility::initCustomKeys(void) { }
void BridgingUtility::addCustomEnvironmentInformation(const char* information, const char* key) { }
void BridgingUtility::setUserIdentifier(const char* userid) { }
void BridgingUtility::setUserName(const char* username) { }
void BridgingUtility::setUserEmail(const char* email) { }
void BridgingUtility::logEvent(const char* ev)
{
	FL_logEvent(ev);
}
void BridgingUtility::logEvent(const char* ev, const std::map<std::string, std::string>& params)
{
	FL_logEvent(ev, params);
}
void BridgingUtility::logTimedEvent(const char* ev) { }
void BridgingUtility::logTimedEvent(const char* ev, const std::map<std::string, std::string>& params) { }
void BridgingUtility::endTimedEvent(const char* ev) { }
void BridgingUtility::endTimedEvent(const char* ev, const std::map<std::string, std::string>& params) { } // non-empty parameters will update the parameters
void BridgingUtility::setCrashContext(const char* value, const char* key) { }
void BridgingUtility::submitFeedback(const char* feedback) { }
void BridgingUtility::forceCrash(void) { }

// Beta UI
void BridgingUtility::showWelcomeView(void) { }
void BridgingUtility::showThanksView(void) { }

// Misc - audio
void BridgingUtility::configureAudioMode(void) { }
void BridgingUtility::setAudioMode(void) { }
bool BridgingUtility::attemptAudioRestart(void) { return true; }
void BridgingUtility::applyDefaultResignActiveBehavior(void) { }
void BridgingUtility::enableBackgroundAudioCompletionListening(bool enable) { }
bool BridgingUtility::didBackGroundAudioComplete(bool reset)
{
	return CocosDenshion::SimpleAudioEngine::sharedEngine()->didBackgroundMusicComplete();
}

// Flurry
void BridgingUtility::FL_startSession(bool reportCrashes) { /* ignore - done in java code */ }
void BridgingUtility::FL_setUserID(const char* userid) { }
void BridgingUtility::FL_logTimedEvent(const char* ev) { }
void BridgingUtility::FL_logTimedEvent(const char* ev, const std::map<std::string, std::string>& params) { }
void BridgingUtility::FL_endTimedEvent(const char* ev) { }
void BridgingUtility::FL_endTimedEvent(const char* ev, const std::map<std::string, std::string>& params) { }

#if FLURRY_ENABLED
void BridgingUtility::FL_logEvent(const char* ev)
{
	CMJniUtils::FL_logEvent(ev);
}
void BridgingUtility::FL_logEvent(const char* ev, const std::map<std::string, std::string>& params)
{
	CMJniUtils::FL_logEvent(ev, params);
}
#else
void BridgingUtility::FL_logEvent(const char* ev) { }
void BridgingUtility::FL_logEvent(const char* ev, const std::map<std::string, std::string>& params) { }
#endif

// Crashlytics
void BridgingUtility::CL_start(void) { }
void BridgingUtility::CL_initCustomKeys(void) { }
void BridgingUtility::CL_setUserIdentifier(const char* userid) { }
void BridgingUtility::CL_setUserName(const char* username) { }
void BridgingUtility::CL_setUserEmail(const char* email) { }
void BridgingUtility::CL_setIntValue(int value, const char* key) { }
void BridgingUtility::CL_setBoolValue(bool value, const char* key) { }
void BridgingUtility::CL_setFloatValue(float value, const char* key) { }
void BridgingUtility::CL_setStringValue(const char* value, const char* key) { }
void BridgingUtility::CL_log(const char* msg) { }
void BridgingUtility::CL_crash(void) { }

// TestFlight
void BridgingUtility::TF_takeOff(void) { }
void BridgingUtility::TF_addCustomEnvironmentInformation(const char* information, const char* key) { }
void BridgingUtility::TF_passCheckpoint(const char* checkpointName) { }
void BridgingUtility::TF_submitFeedback(const char* feedback) { }

// Misc
void BridgingUtility::onBackgroundAudioCompleted(void) { }
