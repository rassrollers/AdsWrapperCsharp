#pragma once

#include <memory>
#include <string>
#include "AdsLib.h"
#include "AdsDevice.h"

class NativeAdsDevice
{
public:
    NativeAdsDevice(const std::string& localIp);

    ~NativeAdsDevice();

    void AddRemoteRoute(const std::string& routeName,
        const std::string& remoteIp,
        uint16_t port,
        const std::string& user,
        const std::string& password);

    AdsDeviceState GetState();

	DeviceInfo GetDeviceInfo();

private:
    std::unique_ptr<AmsNetId> _localAms;
    std::unique_ptr<AmsNetId> _remoteAms;
    std::unique_ptr<AdsDevice> _device;

    std::string _localIp;
    std::string _remoteIp;
};
