#include "pch.h"
#include "NativeAdsDevice.h"
#include "NativeLogger.h"

NativeAdsDevice::NativeAdsDevice(const std::string& localIp, const std::string& localNetId)
{
	NativeLogger::Instance().Log(NativeLogger::LogLevel::Info, "Setting local AMS NetId to " + localNetId);
    _localIp = localIp;
    _localAms = std::make_unique<AmsNetId>(localNetId);
    bhf::ads::SetLocalAddress(*_localAms);
}

NativeAdsDevice::~NativeAdsDevice() = default;

void NativeAdsDevice::AddRemoteRoute(const std::string& routeName,
    const std::string& remoteIp,
	const std::string& remoteNetId,
    uint16_t amsPort,
    const std::string& user,
    const std::string& password)
{
    _remoteIp = remoteIp;
    _remoteAms = std::make_unique<AmsNetId>(remoteNetId);

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
        "Adding remote route to " + _remoteIp);

    long routeStatus = bhf::ads::AddRemoteRoute(_remoteIp,
        *_localAms,
        _localIp,
        routeName,
        user,
        password);

    if (routeStatus) {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeAdsDevice: AddRemoteRoute failed with error code " + std::to_string(routeStatus));
        throw AdsException(routeStatus);
    }

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
        "Creating AdsDevice for remote IP " + remoteIp + " on port " + std::to_string(amsPort));
    _device = std::make_unique<AdsDevice>(_remoteIp,
        *_remoteAms,
        amsPort);

	_device->SetTimeout(5000); // Set a default timeout of 5 seconds
}

void NativeAdsDevice::GetRemoteNetId(const std::string& remoteIp, std::string& netId)
{
    AmsNetId amsNetId;
    long status = bhf::ads::GetRemoteAddress(remoteIp, amsNetId);
    if (status) {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeAdsDevice: GetRemoteNetId failed with error code " + std::to_string(status));
        throw AdsException(status);
    }
	netId = AdsWrapper::AmsToString(amsNetId);
}

void NativeAdsDevice::SetTwinCatState(ADSSTATE adsState, ADSSTATE deviceState)
{
    if (!_device) {
        throw std::runtime_error("AdsDevice not initialized. Call AddRemoteRoute first.");
    }
    
    _device->SetState(adsState, deviceState);
}

AdsDeviceState NativeAdsDevice::GetState() const
{
    if (!_device) {
        throw std::runtime_error("AdsDevice not initialized. Call AddRemoteRoute first.");
    }
    return _device->GetState();
}

DeviceInfo NativeAdsDevice::GetDeviceInfo() const
{
    if (!_device) {
        throw std::runtime_error("AdsDevice not initialized. Call AddRemoteRoute first.");
    }
    return _device->GetDeviceInfo();
}

void NativeAdsDevice::ReadSymbol(const std::string& symbolName, void* buffer, size_t bufferSize) const
{
    if (!_device) {
        throw std::runtime_error("AdsDevice not initialized. Call AddRemoteRoute first.");
    }

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
        "NativeAdsDevice: ReadSymbol '" + symbolName + "' size=" + std::to_string(bufferSize));

    AdsHandle handle = _device->GetHandle(symbolName);

    uint32_t bytesRead = 0;
    long error = _device->ReadReqEx2(ADSIGRP_SYM_VALBYHND, *handle, bufferSize, buffer, &bytesRead);

    if (error || bytesRead != bufferSize) {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeAdsDevice: ReadSymbol failed for '" + symbolName +
            "' error=" + std::to_string(error) + " bytesRead=" + std::to_string(bytesRead));
        throw AdsException(error);
    }

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
        "NativeAdsDevice: ReadSymbol '" + symbolName + "' read " + std::to_string(bytesRead) + " bytes");
}

void NativeAdsDevice::WriteSymbol(const std::string& symbolName, const void* buffer, size_t bufferSize) const
{
    if (!_device) {
        throw std::runtime_error("AdsDevice not initialized. Call AddRemoteRoute first.");
    }

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
        "NativeAdsDevice: WriteSymbol '" + symbolName + "' size=" + std::to_string(bufferSize));

    AdsHandle handle = _device->GetHandle(symbolName);

    long error = _device->WriteReqEx(ADSIGRP_SYM_VALBYHND, *handle, bufferSize, buffer);

    if (error) {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeAdsDevice: WriteSymbol failed for '" + symbolName +
            "' error=" + std::to_string(error));
        throw AdsException(error);
    }

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
        "NativeAdsDevice: WriteSymbol '" + symbolName + "' completed successfully");
}
