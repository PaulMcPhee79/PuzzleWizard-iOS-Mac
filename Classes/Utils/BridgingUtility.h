#ifndef __BRIDGING_UTILITY_H__
#define __BRIDGING_UTILITY_H__

#include <string>
#include <vector>
#include <map>

class IEventListener;
struct vec2;

typedef std::map<std::string, std::string> EvLogParamKV; // Key-Value pairs in that order

class BridgingUtility
{
public:
    static bool isMainThread(void);
	static bool isFullscreen(void);
    static void setFullscreen(bool value);
    static bool isIOSFeatureSupported(const char* reqSysVer);
    static bool shouldPromptForRating(void);
    static void userRespondedToRatingPrompt(bool response);
    static void userDidRateTheGame(bool success);
    static std::vector<std::string> getPreferredLanguageCodes(void);
    static std::string getSplashImagePath(void);
    static bool isHighPerformanceMachine(void);
    static bool isPad(void);
    static void postNotification(const char* name);
    static void openURL(const char* url);
    static void openFileURL(const char* url);
    static void setIdleTimerDisabled(bool disabled);
    static void setStatusBarHidden(bool hidden);
    static void enableStatusBarLightColorStyle(bool enable);
    static std::string localizedString(const char* str);
    static int alertButtonReturnToZeroBasedIndex(int value);
    static void showAlertView(IEventListener* listener, int tag, const std::vector<std::string>& config, bool cancelButton = false);

    // Google Play Licensing
#ifdef __ANDROID__
    static const int kGooglePlayLicenseInProgress = -1;
    static const int kGooglePlayLicenseDenied = 0;
    static const int kGooglePlayLicenseApproved = 1;
    static const int kGooglePlayLicenseProvisionalRetry = 2;
    static const int kGooglePlayLicenseProvisionalError = 3;
	static const int kGooglePlayLicenseDialogOK = 4;
	static const int kGooglePlayLicenseDialogRetry = 5;
	static const int kGooglePlayLicenseDialogBuy = 6;
    static void doGooglePlayLicenseCheck(void);
    static int getGooglePlayLicenseStatus(void);
    static void setGooglePlayLicenseExpiration(int minsRemaining);
    static bool verifyGooglePlayLicenseKey(const char* licenseKey);
    static std::string getGooglePlayLicenseKey(void);
    static void showGooglePlayLicenseDialog(const char* dialogType);
#endif

    // Common public interface
    static const char* CRASH_CONTEXT_GAME_STATE;        // LevelMenu, PuzzleMenu, Playing
    static const char* CRASH_CONTEXT_LEVEL_NAME;        // First Steps, Color Shield, Color Magic, etc
    static const char* CRASH_CONTEXT_PUZZLE_NAME;       // Yellow Brick Road, Apple, Time Travel, etc
    static const char* CRASH_CONTEXT_LOCALE;            // EN, DE, IT, etc
    static const char* CRASH_CONTEXT_MENU;              // MenuEsc, PlayfieldEsc, Options, etc
    static const char* CRASH_CONTEXT_SFX;               // CONTEXT_ENABLED, CONTEXT_DISABLED
    static const char* CRASH_CONTEXT_MUSIC;             // CONTEXT_ENABLED, CONTEXT_DISABLED
    static const char* CRASH_CONTEXT_ICLOUD;            // CONTEXT_ENABLED, CONTEXT_DISABLED
    static const char* CRASH_CONTEXT_COLOR_BLIND;       // CONTEXT_ENABLED, CONTEXT_DISABLED
    static const char* CONTEXT_ENABLED;                 // Enabled
    static const char* CONTEXT_DISABLED;                // Disabled
    static const char* bool2Context(bool value);
    
    static const char* EV_NAME_LOCALE_CHANGED;
    static const char* EV_NAME_LEVEL_UNLOCKED;
    static const char* EV_NAME_LEVEL_COMPLETED;
    static const char* EV_NAME_PUZZLE_ATTEMPTED;
    static const char* EV_NAME_PUZZLE_SOLVED;
    static const char* EV_NAME_RATED_THE_GAME;
    // ****************** BETA *********************
    static const char* EV_NAME_BETA_PUZZLE_BUGGED;
    static const char* EV_NAME_BETA_SUGGEST_IQ_;
    // *********************************************
    
    static void startSession(bool reportCrashes);
    static void initCustomKeys(void);
    static void addCustomEnvironmentInformation(const char* information, const char* key);
    static void setUserIdentifier(const char* userid);
    static void setUserName(const char* username);
    static void setUserEmail(const char* email);
    static void logEvent(const char* ev);
    static void logEvent(const char* ev, const std::map<std::string, std::string>& params);
    static void logTimedEvent(const char* ev);
    static void logTimedEvent(const char* ev, const std::map<std::string, std::string>& params);
    static void endTimedEvent(const char* ev);
    static void endTimedEvent(const char* ev, const std::map<std::string, std::string>& params); // non-empty parameters will update the parameters
    static void setCrashContext(const char* value, const char* key);
    static void submitFeedback(const char* feedback);
    static void forceCrash(void);
    
    // Beta UI
    static void showWelcomeView(void);
    static void showThanksView(void);
    
    // Misc - audio
    static void configureAudioMode(void);
    static void setAudioMode(void);
    static bool attemptAudioRestart(void);
    static void applyDefaultResignActiveBehavior(void);
    static void enableBackgroundAudioCompletionListening(bool enable);
    static bool didBackGroundAudioComplete(bool reset = true);
    
    // Misc - Pause/Resume
    static void pauseEngine(void);
    static bool pauseEngine(const char* lock);
    static void resumeEngine(void);
    static bool resumeEngine(const char* unlock);
    
private:
    // Flurry
    static void FL_startSession(bool reportCrashes);
    static void FL_setUserID(const char* userid);
    static void FL_logEvent(const char* ev);
    static void FL_logEvent(const char* ev, const std::map<std::string, std::string>& params);
    static void FL_logTimedEvent(const char* ev);
    static void FL_logTimedEvent(const char* ev, const std::map<std::string, std::string>& params);
    static void FL_endTimedEvent(const char* ev);
    static void FL_endTimedEvent(const char* ev, const std::map<std::string, std::string>& params);
    
    // Crashlytics
    static void CL_start(void);
    static void CL_initCustomKeys(void);
    static void CL_setUserIdentifier(const char* userid);
    static void CL_setUserName(const char* username);
    static void CL_setUserEmail(const char* email);
    static void CL_setIntValue(int value, const char* key);
    static void CL_setBoolValue(bool value, const char* key);
    static void CL_setFloatValue(float value, const char* key);
    static void CL_setStringValue(const char* value, const char* key);
    static void CL_log(const char* msg);
    static void CL_crash(void);
    
    // TestFlight
    static void TF_takeOff(void);
    static void TF_addCustomEnvironmentInformation(const char* information, const char* key);
    static void TF_passCheckpoint(const char* checkpointName);
    static void TF_submitFeedback(const char* feedback);
    
    // Misc
    static void onBackgroundAudioCompleted(void);

    BridgingUtility(void);
	~BridgingUtility(void);
};
#endif // __BRIDGING_UTILITY_H__
