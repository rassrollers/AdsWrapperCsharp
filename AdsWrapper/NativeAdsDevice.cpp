#include "pch.h"
#include "NativeAdsDevice.h"
#include "NativeLogger.h"

NativeAdsDevice::NativeAdsDevice(const std::string& localIp)
{
	NativeLogger::Instance().Log(NativeLogger::LogLevel::Info, "Setting local AMS NetId to " + localIp + ".1.1");
    _localIp = localIp;
    _localAms = std::make_unique<AmsNetId>(localIp + ".1.1");
    bhf::ads::SetLocalAddress(*_localAms);
}

NativeAdsDevice::~NativeAdsDevice() = default;

void NativeAdsDevice::AddRemoteRoute(const std::string& routeName,
    const std::string& remoteIp,
    uint16_t port,
    const std::string& user,
    const std::string& password)
{
    _remoteIp = remoteIp;

    // Query the actual AmsNetId from the remote IPC instead of assuming .1.1
    AmsNetId remoteNetId;
    long getAddrStatus = bhf::ads::GetRemoteAddress(_remoteIp, remoteNetId);
    if (getAddrStatus) {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeAdsDevice: GetRemoteAddress failed with error code " + std::to_string(getAddrStatus));
        throw AdsException(getAddrStatus);
    }
    NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
        "Retrieved remote AMS NetId for " + _remoteIp + ": " +
        std::to_string(remoteNetId.b[0]) + "." +
        std::to_string(remoteNetId.b[1]) + "." +
        std::to_string(remoteNetId.b[2]) + "." +
        std::to_string(remoteNetId.b[3]) + "." +
        std::to_string(remoteNetId.b[4]) + "." +
		std::to_string(remoteNetId.b[5]));
    _remoteAms = std::make_unique<AmsNetId>(remoteNetId);

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
        "Adding remote route to " + _remoteIp);

    long routeStatus = bhf::ads::AddRemoteRoute(_remoteIp,
        *_remoteAms,
        _remoteIp,
        routeName,
        user,
        password);

    if (routeStatus) {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeAdsDevice: AddRemoteRoute failed with error code " + std::to_string(routeStatus));
        throw AdsException(routeStatus);
    }

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
        "Creating AdsDevice for remote IP " + remoteIp + " on port " + std::to_string(port));
    _device = std::make_unique<AdsDevice>(_remoteIp,
        *_remoteAms,
        port);
}

AdsDeviceState NativeAdsDevice::GetState()
{
    if (!_device) {
        throw std::runtime_error("AdsDevice not initialized. Call AddRemoteRoute first.");
    }
    return _device->GetState();
}

DeviceInfo NativeAdsDevice::GetDeviceInfo()
{
    if (!_device) {
        throw std::runtime_error("AdsDevice not initialized. Call AddRemoteRoute first.");
    }
    return _device->GetDeviceInfo();
}
