#include "pch.h"
#include "AdsDeviceWrapper.h"
#include "NativeAdsDevice.h"
#include "NativeLogger.h"

#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

namespace AdsWrapper
{
    AdsDeviceWrapper::AdsDeviceWrapper(String^ localIp, String^ localNetId)
    {
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: Creating new instance");

            std::string lIp = marshal_as<std::string>(localIp);
            std::string lNetId = marshal_as<std::string>(localNetId);

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "AdsDeviceWrapper: LocalIp=" + lIp);

            _native = new NativeAdsDevice(lIp, lNetId);

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: Instance created successfully");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: Failed to create instance - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    AdsDeviceWrapper::~AdsDeviceWrapper()
    {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
            "AdsDeviceWrapper: Disposing");
        this->!AdsDeviceWrapper();
        GC::SuppressFinalize(this);
    }

    AdsDeviceWrapper::!AdsDeviceWrapper()
    {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
            "AdsDeviceWrapper: Releasing native resources");
        delete _native;
        _native = nullptr;
    }

    void AdsDeviceWrapper::AddRemoteRoute(String^ routeName,
        String^ remoteIp,
		String^ remoteNetId,
        UInt16 amsPort, 
        String^ user, 
        String^ password)
    {
        try
        {
            std::string rName = marshal_as<std::string>(routeName);
            std::string rIp = marshal_as<std::string>(remoteIp);
			std::string rNetId = marshal_as<std::string>(remoteNetId);
            std::string u = marshal_as<std::string>(user);
            std::string p = marshal_as<std::string>(password);

            _native->AddRemoteRoute(rName, rIp, rNetId, amsPort, u, p);

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: AddRemoteRoute completed successfully");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: AddRemoteRoute failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    void AdsDeviceWrapper::SetTwinCatState(AdsState adsState, AdsState deviceState)
    {
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "AdsDeviceWrapper: SetTwinCatState called with AdsState=" + std::to_string(static_cast<int>(adsState)) +
                ", DeviceState=" + std::to_string(static_cast<int>(deviceState)));
            _native->SetTwinCatState(static_cast<ADSSTATE>(adsState), static_cast<ADSSTATE>(deviceState));
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: SetTwinCatState completed successfully");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: SetTwinCatState failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
		}
    }

    StateInfo AdsDeviceWrapper::GetState()
    {
        try
        {
            auto nativeState = _native->GetState();

            StateInfo state;
            state.Ads = static_cast<AdsState>(nativeState.ads);
            state.Device = static_cast<AdsState>(nativeState.device);

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "AdsDeviceWrapper: GetState returned AdsState=" + std::to_string(static_cast<int>(state.Ads)) +
                ", DeviceState=" + std::to_string(static_cast<int>(state.Device)));

            return state;
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: GetState failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    DeviceInfo AdsDeviceWrapper::GetDeviceInfo()
    {
        try
        {
			auto info = _native->GetDeviceInfo();

			DeviceInfo managedInfo;
			managedInfo.Name = gcnew String(info.name);
			managedInfo.Version = info.version.version;
			managedInfo.Revision = info.version.revision;
			managedInfo.Build = info.version.build;

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "AdsDeviceWrapper: GetDeviceInfo returned Name=" + marshal_as<std::string>(managedInfo.Name) +
                ", Version=" + std::to_string(managedInfo.Version) +
                ", Revision=" + std::to_string(managedInfo.Revision) +
				", Build=" + std::to_string(managedInfo.Build));

			return managedInfo;
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: GetDeviceInfo failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }
}
