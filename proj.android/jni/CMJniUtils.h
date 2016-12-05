/*
 * CMJniUtils.h
 *
 *  Created on: Jul 31, 2013
 *      Author: paulmcphee
 */

#ifndef CMJNIUTILS_H_
#define CMJNIUTILS_H_

#include <Utils/Localizer.h>

class CMJniUtils {
public:
	static void openMarketPlaceToRateTheGame(void);
	static Localizer::LocaleType getLocaleTypeFromCurrentUICulture(void);
	static int getDisplayWidthInPixels(void);
	static int getDisplayHeightInPixels(void);
	static float getDisplayXdpi(void);
	static float getDisplayYdpi(void);
	static float getDisplayWidthInInches(void);
	static float getDisplayHeightInInches(void);
	static std::string getResStringForKey(const char* key);

	static void setDirtyRenderMode(int fps);

	static void enableKeepScreenOn(bool enable);
	static void backupGameProgress(void);
	static void openURL(const char* url);

	// Licensing
	static void doGooglePlayLicenseCheck(void);
	static int getGooglePlayLicenseStatus(void);
	static void setGooglePlayLicenseExpiration(int minsRemaining);
    static bool verifyGooglePlayLicenseKey(const char* licenseKey);
    static std::string getGooglePlayLicenseKey(void);
    static void showGooglePlayLicenseDialog(const char* dialogType);

	// Flurry
	static void FL_logEvent(const char* ev);
	static void FL_logEvent(const char* ev, const std::map<std::string, std::string>& params);

private:
	static bool getBool(const char* methodName);
	static int getInt(const char* methodName);
	static float getFloat(const char* methodName);
	static std::string getString(const char* methodName);
	static std::string getString(const char* methodName, const char* strParam);
	static void callStaticVoidMethod(const char* className, const char* methodName);
	static void callStaticIntMethod(const char* className, const char* methodName, int intParam);
	static void callStaticStringMethod(const char* className, const char* methodName, const char* strParam);
};

#endif /* CMJNIUTILS_H_ */
