#include "pch.h"
#include "LicenseAccessWrapper.h"
#include "NativeLicenseAccess.h"
#include "NativeLogger.h"

#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

namespace AdsWrapper
{
    LicenseAccessWrapper::LicenseAccessWrapper(String^ remoteIp, String^ remoteNetId)
        : LicenseAccessWrapper(remoteIp, remoteNetId, 30)
    {
    }

    LicenseAccessWrapper::LicenseAccessWrapper(String^ remoteIp, String^ remoteNetId, uint16_t port)
    {
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: Creating new instance");

            std::string rIp = marshal_as<std::string>(remoteIp);
            std::string rNetId = marshal_as<std::string>(remoteNetId);

            _native = new NativeLicenseAccess(rIp, rNetId, port);

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: Instance created successfully");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "LicenseAccessWrapper: Failed to create instance - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    LicenseAccessWrapper::~LicenseAccessWrapper()
    {
        if (_disposed)
            return;

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
            "LicenseAccessWrapper: Disposing");
        this->!LicenseAccessWrapper();
        GC::SuppressFinalize(this);
        _disposed = true;
    }

    LicenseAccessWrapper::!LicenseAccessWrapper()
    {
        if (_native != nullptr)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: Releasing native resources");
            delete _native;
            _native = nullptr;
        }
    }

    void LicenseAccessWrapper::CheckDisposed()
    {
        if (_disposed)
            throw gcnew ObjectDisposedException("LicenseAccessWrapper");
    }

    String^ LicenseAccessWrapper::GetOnlineInfo()
    {
        CheckDisposed();
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: GetOnlineInfo called");

            std::string info = _native->GetOnlineInfo();
            
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: GetOnlineInfo completed successfully");

            return gcnew String(info.c_str());
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "LicenseAccessWrapper: GetOnlineInfo failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    uint16_t LicenseAccessWrapper::GetPlatformId()
    {
        CheckDisposed();
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: GetPlatformId called");

            uint16_t platformId = _native->GetPlatformId();

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: GetPlatformId completed successfully, value: " + std::to_string(platformId));

            return platformId;
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "LicenseAccessWrapper: GetPlatformId failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    String^ LicenseAccessWrapper::GetSystemId()
    {
        CheckDisposed();
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: GetSystemId called");

            std::string systemId = _native->GetSystemId();

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: GetSystemId completed successfully");

            return gcnew String(systemId.c_str());
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "LicenseAccessWrapper: GetSystemId failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    uint32_t LicenseAccessWrapper::GetVolumeNo()
    {
        CheckDisposed();
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: GetVolumeNo called");

            uint32_t volumeNo = _native->GetVolumeNo();

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "LicenseAccessWrapper: GetVolumeNo completed successfully, value: " + std::to_string(volumeNo));

            return volumeNo;
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "LicenseAccessWrapper: GetVolumeNo failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }
}
