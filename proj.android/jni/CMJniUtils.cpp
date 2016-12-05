/*
 * CMJniUtils.cpp
 *
 *  Created on: Jul 31, 2013
 *      Author: paulmcphee
 */

#include "CMJniUtils.h"
#include "cocos2d.h"
#include "../platform/android/jni/JniHelper.h"

#define CMJNI_CLASS_NAME "com/cheekymammoth/puzzlewizard/puzzlewizard"

typedef std::map<std::string, std::string> FL_ParamKV; // Key-Value pairs in that order

USING_NS_CC;

bool CMJniUtils::getBool(const char* methodName)
{
	JniMethodInfo methodInfo;
	bool ret = 0;

	if (!JniHelper::getStaticMethodInfo(methodInfo, CMJNI_CLASS_NAME, methodName, "()Z"))
		return ret;

	ret = methodInfo.env->CallStaticIntMethod(methodInfo.classID, methodInfo.methodID);
	methodInfo.env->DeleteLocalRef(methodInfo.classID);

	return ret;
}

int CMJniUtils::getInt(const char* methodName)
{
	JniMethodInfo methodInfo;
	int ret = 0;

	if (!JniHelper::getStaticMethodInfo(methodInfo, CMJNI_CLASS_NAME, methodName, "()I"))
		return ret;

	ret = methodInfo.env->CallStaticIntMethod(methodInfo.classID, methodInfo.methodID);
	methodInfo.env->DeleteLocalRef(methodInfo.classID);

	return ret;
}

float CMJniUtils::getFloat(const char* methodName)
{
	JniMethodInfo methodInfo;
	float ret = 0;

	if (!JniHelper::getStaticMethodInfo(methodInfo, CMJNI_CLASS_NAME, methodName, "()F"))
		return ret;

	ret = methodInfo.env->CallStaticFloatMethod(methodInfo.classID, methodInfo.methodID);
	methodInfo.env->DeleteLocalRef(methodInfo.classID);

	return ret;
}

std::string CMJniUtils::getString(const char* methodName)
{
	JniMethodInfo methodInfo;
	std::string ret("");

	if (!JniHelper::getStaticMethodInfo(methodInfo, CMJNI_CLASS_NAME, methodName, "()Ljava/lang/String;"))
		return ret;

	jstring j_str = (jstring)methodInfo.env->CallStaticObjectMethod(methodInfo.classID, methodInfo.methodID);
	methodInfo.env->DeleteLocalRef(methodInfo.classID);
	ret = JniHelper::jstring2string(j_str);
	if (j_str != NULL)
		methodInfo.env->DeleteLocalRef(j_str);
	return ret;
}

std::string CMJniUtils::getString(const char* methodName, const char* strParam)
{
	JniMethodInfo methodInfo;
	std::string ret("");

	if (!JniHelper::getStaticMethodInfo(methodInfo, CMJNI_CLASS_NAME, methodName, "(Ljava/lang/String;)Ljava/lang/String;"))
		return ret;

	jstring j_strParam = methodInfo.env->NewStringUTF(strParam);
	jstring j_str = (jstring)methodInfo.env->CallStaticObjectMethod(methodInfo.classID, methodInfo.methodID, j_strParam);
	methodInfo.env->DeleteLocalRef(j_strParam);
	methodInfo.env->DeleteLocalRef(methodInfo.classID);
	ret = JniHelper::jstring2string(j_str);
	if (j_str != NULL)
		methodInfo.env->DeleteLocalRef(j_str);
	return ret;
}

void CMJniUtils::callStaticVoidMethod(const char* className, const char* methodName)
{
	JniMethodInfo methodInfo;

	if (JniHelper::getStaticMethodInfo(methodInfo, className, methodName, "()V"))
	{
		methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID);
		methodInfo.env->DeleteLocalRef(methodInfo.classID);
	}
}

void CMJniUtils::callStaticIntMethod(const char* className, const char* methodName, int intParam)
{
	JniMethodInfo methodInfo;

	if (JniHelper::getStaticMethodInfo(methodInfo, className, methodName, "(I)V"))
	{
		methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, intParam);
		methodInfo.env->DeleteLocalRef(methodInfo.classID);
	}
}

void CMJniUtils::callStaticStringMethod(const char* className, const char* methodName, const char* strParam)
{
	JniMethodInfo methodInfo;

	if (JniHelper::getStaticMethodInfo(methodInfo, className, methodName, "(Ljava/lang/String;)V"))
	{
		jstring j_strParam = methodInfo.env->NewStringUTF(strParam);
		methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, j_strParam);
		methodInfo.env->DeleteLocalRef(j_strParam);
		methodInfo.env->DeleteLocalRef(methodInfo.classID);
	}
}

void CMJniUtils::openMarketPlaceToRateTheGame(void)
{
	callStaticVoidMethod(CMJNI_CLASS_NAME, "openMarketPlaceToRateTheGame");
}

Localizer::LocaleType CMJniUtils::getLocaleTypeFromCurrentUICulture(void)
{
	int localeInt = getInt("getDefaultLocale");
	if (localeInt >= (int)Localizer::EN && localeInt < Localizer::INVALID_LOCALE)
		return (Localizer::LocaleType)localeInt;
	else
		return Localizer::EN;
}

int CMJniUtils::getDisplayWidthInPixels(void)
{
	return getInt("getDisplayWidthInPixels");
}

int CMJniUtils::getDisplayHeightInPixels(void)
{
	return getInt("getDisplayHeightInPixels");
}

float CMJniUtils::getDisplayXdpi(void)
{
	return getFloat("getDisplayXdpi");
}

float CMJniUtils::getDisplayYdpi(void)
{
	return getFloat("getDisplayYdpi");
}

float CMJniUtils::getDisplayWidthInInches(void)
{
	int w = getDisplayWidthInPixels();
	float dpi = getDisplayXdpi();

	return w == 0 || dpi < 0.01f ? 0 : w / dpi;
}

float CMJniUtils::getDisplayHeightInInches(void)
{
	int h = getDisplayHeightInPixels();
	float dpi = getDisplayYdpi();

	return h == 0 || dpi < 0.01f ? 0 : h / dpi;
}

std::string CMJniUtils::getResStringForKey(const char* key)
{
	if (key)
	{
		std::string keyStr(key);

		if (keyStr.compare("CM_LocaleKey") == 0)
			return getString("getLocaleFromResStrings");
		//else if ... add further keys here
	}

	return "";
}

void CMJniUtils::setDirtyRenderMode(int fps)
{
	if (fps == 20)
		callStaticVoidMethod("org/cocos2dx/lib/Cocos2dxGLSurfaceView", "Cocos2dxSetDirtyRender20");
	else if (fps == 30)
		callStaticVoidMethod("org/cocos2dx/lib/Cocos2dxGLSurfaceView", "Cocos2dxSetDirtyRender30");
	else
		CCLog("CMJniUtils::setDirtyRenderMode - invalid FPS setting. Ignoring call.");
}

void CMJniUtils::enableKeepScreenOn(bool enable)
{
	if (enable)
		callStaticVoidMethod(CMJNI_CLASS_NAME, "keepScreenOn");
	else
		callStaticVoidMethod(CMJNI_CLASS_NAME, "allowScreenOff");
}

void CMJniUtils::backupGameProgress(void)
{
	callStaticVoidMethod(CMJNI_CLASS_NAME, "backupGameProgress");
}

void CMJniUtils::openURL(const char* url)
{
	if (url)
		callStaticStringMethod(CMJNI_CLASS_NAME, "openURL", url);
}

// Licensing
void CMJniUtils::doGooglePlayLicenseCheck(void)
{
	callStaticVoidMethod(CMJNI_CLASS_NAME, "congratulateUser");
}

int CMJniUtils::getGooglePlayLicenseStatus(void)
{
	return getInt("getPuzzleCompletedCount");
}

void CMJniUtils::setGooglePlayLicenseExpiration(int minsRemaining)
{
	callStaticIntMethod(CMJNI_CLASS_NAME, "setStatLongestPuzzleDuration", minsRemaining);
}

bool CMJniUtils::verifyGooglePlayLicenseKey(const char* licenseKey)
{
	JniMethodInfo methodInfo;
	bool ret = true; // Return success by default. Don't let errors prevent users from playing.

	if (!JniHelper::getStaticMethodInfo(methodInfo, CMJNI_CLASS_NAME, "isPlayerMasterPuzzler", "(Ljava/lang/String;)Z"))
		return ret;

	jstring j_licenseKey = licenseKey ? methodInfo.env->NewStringUTF(licenseKey) : methodInfo.env->NewStringUTF("");
	ret = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID, j_licenseKey);
	methodInfo.env->DeleteLocalRef(j_licenseKey);
	methodInfo.env->DeleteLocalRef(methodInfo.classID);

	return ret;
}

std::string CMJniUtils::getGooglePlayLicenseKey(void)
{
	return getString("getMasterPuzzlerKey");
}

void CMJniUtils::showGooglePlayLicenseDialog(const char* dialogType)
{
	if (dialogType)
		callStaticStringMethod(CMJNI_CLASS_NAME, "showGooglePlayLicenseDialog", dialogType);
}

//void CMJniUtils::backupToCloud(const char* filename, const char* key)
//{
//	if (filename && key)
//	{
//		JniMethodInfo methodInfo;
//		if (JniHelper::getStaticMethodInfo(methodInfo, CMJNI_CLASS_NAME, "backupToCloud", "(Ljava/lang/String;Ljava/lang/String;)V"))
//		{
//			jstring j_filename = methodInfo.env->NewStringUTF(filename);
//			jstring j_key = methodInfo.env->NewStringUTF(key);
//			methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, j_filename, j_key);
//			methodInfo.env->DeleteLocalRef(j_filename);
//			methodInfo.env->DeleteLocalRef(j_key);
//			methodInfo.env->DeleteLocalRef(methodInfo.classID);
//		}
//	}
//}

// Flurry
void CMJniUtils::FL_logEvent(const char* ev)
{
	if (ev)
		callStaticStringMethod(CMJNI_CLASS_NAME, "FL_logEvent", ev);
}

void CMJniUtils::FL_logEvent(const char* ev, const std::map<std::string, std::string>& params)
{
	if (ev && params.size() > 0)
	{
		JniMethodInfo methodInfo;
		if (JniHelper::getStaticMethodInfo(methodInfo, CMJNI_CLASS_NAME, "FL_logEvent", "(Ljava/lang/String;[Ljava/lang/String;)V"))
		{
			jobjectArray j_arr = (jobjectArray)methodInfo.env->NewObjectArray(
					2 * params.size(), // x2 due to storing Key,Value tuple
					methodInfo.env->FindClass("java/lang/String"),
					0);

			int i = 0;
			for (FL_ParamKV::const_iterator it = params.begin(); it != params.end(); ++it, i+=2)
			{
				jstring j_key = methodInfo.env->NewStringUTF(it->first.c_str());
				jstring j_value = methodInfo.env->NewStringUTF(it->second.c_str());
				methodInfo.env->SetObjectArrayElement(j_arr, i, j_key);
				methodInfo.env->SetObjectArrayElement(j_arr, i+1, j_value);
				methodInfo.env->DeleteLocalRef(j_key);
				methodInfo.env->DeleteLocalRef(j_value);
			}

			jstring j_ev = methodInfo.env->NewStringUTF(ev);
			methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, j_ev, j_arr);
			methodInfo.env->DeleteLocalRef(j_ev);
			methodInfo.env->DeleteLocalRef(j_arr);
			methodInfo.env->DeleteLocalRef(methodInfo.classID);
		}
	}
}
