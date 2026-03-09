#pragma once

#include <memory>
#include <string>
#include "AdsLib.h"
#include "AdsDevice.h"

class NativeAdsDevice
{
public:
    NativeAdsDevice(const std::string& localIp, const std::string& localNetId);

    ~NativeAdsDevice();

    void AddRemoteRoute(const std::string& routeName,
        const std::string& remoteIp,
		const std::string& remoteNetId,
        uint16_t amsPort,
        const std::string& user,
        const std::string& password);

	void SetTwinCatState(ADSSTATE adsState, ADSSTATE deviceState);
    AdsDeviceState GetState();
	DeviceInfo GetDeviceInfo();

private:
    std::unique_ptr<AmsNetId> _localAms;
    std::unique_ptr<AmsNetId> _remoteAms;
    std::unique_ptr<AdsDevice> _device;

    std::string _localIp;
    std::string _remoteIp;
};
