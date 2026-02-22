#include "pch.h"
#include "NativeAdsDevice.h"
#include "NativeLogger.h"

NativeAdsDevice::NativeAdsDevice(const std::string& localIp,
    const std::string& remoteIp,
    uint16_t port,
    const std::string& routeName,
    const std::string& user,
    const std::string& password)
{
	NativeLogger::Instance().Log(NativeLogger::LogLevel::Info, "Setting local AMS NetId to " + localIp + ".1.1");
    _localAms = std::make_unique<AmsNetId>(localIp + ".1.1");
    bhf::ads::SetLocalAddress(*_localAms);

	NativeLogger::Instance().Log(NativeLogger::LogLevel::Info, "Adding remote route to " + remoteIp + " with AMS NetId " + remoteIp + ".1.1");
    _remoteIp = remoteIp;
    _remoteAms = std::make_unique<AmsNetId>(remoteIp + ".1.1");
    bhf::ads::AddRemoteRoute(remoteIp,
        *_remoteAms,
        remoteIp,
        routeName,
        user,
        password);

	NativeLogger::Instance().Log(NativeLogger::LogLevel::Info, "Creating AdsDevice for remote IP " + remoteIp + " on port " + std::to_string(port));
    _device = std::make_unique<AdsDevice>(_remoteIp,
        *_remoteAms,
        port);
}

NativeAdsDevice::~NativeAdsDevice() = default;

AdsDeviceState NativeAdsDevice::GetState()
{
    return _device->GetState();
}
