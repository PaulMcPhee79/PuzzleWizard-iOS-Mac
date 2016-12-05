
#include "CloudWrapper.h"
#include <Interfaces/IEventListener.h>
#include <Puzzle/Data/GameProgress.h>
#include <CMJniUtils.h>

bool CloudWrapper::isCloudApproved(void) { return false; }
void CloudWrapper::setCloudApproved(bool value) { /* Do nothing */ }
bool CloudWrapper::isCloudSupported(void) { return false; }
void CloudWrapper::enableCloud(bool enable) { /* Do nothing */ }
const void* CloudWrapper::load(ulong& size) { return NULL; }
void CloudWrapper::save(const void* data, ulong size)
{
//	std::string basePath = CCFileUtils::sharedFileUtils()->getWritablePath();
//	std::string defaultPath = basePath + GameProgress::kFilePathExt;
//	CMJniUtils::backupToCloud(defaultPath.c_str(), "PW_GameProgress");
	CMJniUtils::backupGameProgress();
}
void CloudWrapper::applicationDidBecomeActive(void) { /* Do nothing */ }
void CloudWrapper::applicationWillResignActive(void) { /* Do nothing */ }
void CloudWrapper::addEventListener(int evType, IEventListener* listener) { /* Do nothing */ }
void CloudWrapper::removeEventListener(int evType, IEventListener* listener) { /* Do nothing */ }
void CloudWrapper::removeEventListeners(int evType) { /* Do nothing */ }
int CloudWrapper::getEvTypeLoggedIn(void) { return -1; }
int CloudWrapper::getEvTypeLoggedOut(void) { return -1; }
int CloudWrapper::getEvTypeDataChanged(void) { return -1; }
int CloudWrapper::getEvTypeSettingsChanged(void) { return -1; }
int CloudWrapper::getEvTypeUnavailable(void) { return -1; }
