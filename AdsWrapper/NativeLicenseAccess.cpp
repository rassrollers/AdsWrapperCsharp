#include "pch.h"
#include "NativeLicenseAccess.h"
#include "NativeLogger.h"
#include <sstream>

NativeLicenseAccess::NativeLicenseAccess(const std::string& remoteIp, const std::string& remoteNetId, uint16_t port)
{
    try
    {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
            "NativeLicenseAccess: Creating License device for remote NetID " + remoteNetId + " on AMS port " + std::to_string(port));

        AmsNetId netId(remoteNetId);
        _license = std::make_unique<bhf::ads::LicenseAccess>(remoteIp, netId, port);

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
            "NativeLicenseAccess: License access object created successfully");
    }
    catch (const std::exception& ex)
    {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeLicenseAccess: Failed to create - " + std::string(ex.what()));
        throw;
    }
}

NativeLicenseAccess::~NativeLicenseAccess() = default;

std::string NativeLicenseAccess::GetOnlineInfo() const
{
    if (!_license)
    {
        throw std::runtime_error("NativeLicenseAccess: License access not initialized");
    }

    try
    {
        std::ostringstream oss;
        _license->ShowOnlineInfo(oss);
        const auto result = oss.str();

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
            "NativeLicenseAccess: GetOnlineInfo completed successfully");

        return result;
    }
    catch (const std::exception& ex)
    {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeLicenseAccess: GetOnlineInfo failed - " + std::string(ex.what()));
        throw std::runtime_error("Failed to read online license info");
    }
}

uint16_t NativeLicenseAccess::GetPlatformId() const
{
    if (!_license)
    {
        throw std::runtime_error("NativeLicenseAccess: License access not initialized");
    }

    try
    {
        std::ostringstream oss;
        _license->ShowPlatformId(oss);
        const auto result = std::stoul(oss.str());

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
            "NativeLicenseAccess: GetPlatformId completed successfully, value: " + std::to_string(result));

        return static_cast<uint16_t>(result);
    }
    catch (const std::exception& ex)
    {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeLicenseAccess: GetPlatformId failed - " + std::string(ex.what()));
        throw std::runtime_error("Failed to read platform ID");
    }
}

std::string NativeLicenseAccess::GetSystemId() const
{
    if (!_license)
    {
        throw std::runtime_error("NativeLicenseAccess: License access not initialized");
    }

    try
    {
        std::ostringstream oss;
        _license->ShowSystemId(oss);
        auto result = oss.str();
        
        // Remove trailing newline
        if (!result.empty() && result.back() == '\n')
        {
            result.pop_back();
        }

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
            "NativeLicenseAccess: GetSystemId completed successfully");

        return result;
    }
    catch (const std::exception& ex)
    {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeLicenseAccess: GetSystemId failed - " + std::string(ex.what()));
        throw std::runtime_error("Failed to read system ID");
    }
}

uint32_t NativeLicenseAccess::GetVolumeNo() const
{
    if (!_license)
    {
        throw std::runtime_error("NativeLicenseAccess: License access not initialized");
    }

    try
    {
        std::ostringstream oss;
        _license->ShowVolumeNo(oss);
        const auto result = std::stoul(oss.str());

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
            "NativeLicenseAccess: GetVolumeNo completed successfully, value: " + std::to_string(result));

        return static_cast<uint32_t>(result);
    }
    catch (const std::exception& ex)
    {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeLicenseAccess: GetVolumeNo failed - " + std::string(ex.what()));
        throw std::runtime_error("Failed to read volume number");
    }
}
