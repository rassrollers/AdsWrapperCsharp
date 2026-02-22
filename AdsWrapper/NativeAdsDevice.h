#pragma once

#include <memory>
#include <string>
#include "AdsLib.h"
#include "AdsDevice.h"

class NativeAdsDevice
{
public:
    NativeAdsDevice(const std::string& localIp,
        const std::string& remoteIp,
        uint16_t port,
        const std::string& routeName,
        const std::string& user,
        const std::string& password);

    ~NativeAdsDevice();

    AdsDeviceState GetState();

private:
    std::unique_ptr<AmsNetId> _localAms;
    std::unique_ptr<AmsNetId> _remoteAms;
    std::unique_ptr<AdsDevice> _device;

    std::string _remoteIp;
};
