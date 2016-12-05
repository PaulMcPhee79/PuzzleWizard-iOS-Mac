
#include "CloudWrapper.h"
#include <Interfaces/IEventListener.h>
#include <Managers/CloudManager.h>

bool CloudWrapper::isCloudApproved(void)
{
    return [CloudManager CM].isCloudApproved;
}

void CloudWrapper::setCloudApproved(bool value)
{
    [CloudManager CM].isCloudApproved = value;
}

bool CloudWrapper::isCloudSupported(void)
{
    return [CloudManager CM].isCloudSupported;
}

bool CloudWrapper::hasActiveCloudAccount(void)
{
    return [CloudManager CM].hasActiveCloudAccount;
}

void CloudWrapper::enableCloud(bool enable)
{
    if (enable)
        [[CloudManager CM] enableCloud];
    else
        [[CloudManager CM] disableCloud];
}

const void* CloudWrapper::load(ulong& size)
{
    NSData *nsData = [[CloudManager CM] load];
    if (nsData)
    {
        size = [nsData length];
        return [nsData bytes];
    }
    else
    {
        size = 0;
        return NULL;
    }
}

void CloudWrapper::save(const void* data, ulong size)
{
    if (data)
    {
        NSData *nsData = [NSData dataWithBytes:data length:size];
        if (nsData)
            [[CloudManager CM] save:nsData];
    }
}

void CloudWrapper::applicationDidBecomeActive(void)
{
    [[CloudManager CM] applicationDidBecomeActive];
}

void CloudWrapper::applicationWillResignActive(void)
{
    [[CloudManager CM] applicationWillResignActive];
}

void CloudWrapper::addEventListener(int evType, IEventListener* listener)
{
    [[CloudManager CM] addEventListener:evType listener:listener];
}

void CloudWrapper::removeEventListener(int evType, IEventListener* listener)
{
    [[CloudManager CM] removeEventListener:evType listener:listener];
}

void CloudWrapper::removeEventListeners(int evType)
{
    [[CloudManager CM] removeEventListeners:evType];
}

int CloudWrapper::getEvTypeLoggedIn(void)
{
    return [CloudManager CUST_EVENT_TYPE_CLOUD_LOGGED_IN];
}

int CloudWrapper::getEvTypeLoggedOut(void)
{
    return [CloudManager CUST_EVENT_TYPE_CLOUD_LOGGED_OUT];
}

int CloudWrapper::getEvTypeDataChanged(void)
{
    return [CloudManager CUST_EVENT_TYPE_CLOUD_DATA_CHANGED];
}

int CloudWrapper::getEvTypeSettingsChanged(void)
{
    return [CloudManager CUST_EVENT_TYPE_CLOUD_SETTINGS_CHANGED];
}

int CloudWrapper::getEvTypeUnavailable(void)
{
    return [CloudManager CUST_EVENT_TYPE_CLOUD_UNAVAILABLE];
}
