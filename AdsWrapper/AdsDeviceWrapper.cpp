#include "pch.h"
#include "AdsDeviceWrapper.h"
#include "NativeAdsDevice.h"
#include "NativeLogger.h"

#include <msclr/marshal_cppstd.h>

// Suppress false IntelliSense errors in C++/CLI generic methods
#ifdef __INTELLISENSE__
#define generic
#define where
#endif

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
        if (_disposed)
            return;

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
            "AdsDeviceWrapper: Disposing");
        this->!AdsDeviceWrapper();
        GC::SuppressFinalize(this);
        _disposed = true;
    }

    AdsDeviceWrapper::!AdsDeviceWrapper()
    {
        if (_native != nullptr)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: Releasing native resources");
            delete _native;
            _native = nullptr;
        }
    }

    void AdsDeviceWrapper::CheckDisposed()
    {
        if (_disposed)
            throw gcnew ObjectDisposedException("AdsDeviceWrapper");
    }

    void AdsDeviceWrapper::AddRemoteRoute(String^ routeName,
        String^ remoteIp,
		String^ remoteNetId,
        AmsPort amsPort, 
        String^ user, 
        String^ password)
    {
        CheckDisposed();
        try
        {
            std::string rName = marshal_as<std::string>(routeName);
            std::string rIp = marshal_as<std::string>(remoteIp);
			std::string rNetId = marshal_as<std::string>(remoteNetId);
            std::string u = marshal_as<std::string>(user);
            std::string p = marshal_as<std::string>(password);

            _native->AddRemoteRoute(rName, rIp, rNetId, static_cast<int>(amsPort), u, p);

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

    String^ AdsDeviceWrapper::GetRemoteNetId(String^ remoteIp)
    {
        CheckDisposed();
        try
        {
            std::string rIp = marshal_as<std::string>(remoteIp);
            std::string rNetId;
            _native->GetRemoteNetId(rIp, rNetId);
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: GetRemoteNetId completed successfully, NetId=" + rNetId);
            return gcnew String(rNetId.c_str());
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: GetRemoteNetId failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
	}

    void AdsDeviceWrapper::SetTwinCatState(AdsState adsState, AdsState deviceState)
    {
        CheckDisposed();
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
        CheckDisposed();
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
        CheckDisposed();
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

    generic <typename T> where T : value class
    T AdsDeviceWrapper::ReadSymbol(String^ symbolName)
    {
        CheckDisposed();
        try
        {
            std::string name = marshal_as<std::string>(symbolName);

            // Marshal::SizeOf<bool> returns 4 (Win32 BOOL), but PLC BOOL is 1 byte
            bool isBool = (T::typeid == Boolean::typeid);
            int size = isBool ? 1 : Marshal::SizeOf(T::typeid);

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "AdsDeviceWrapper: ReadSymbol<" + marshal_as<std::string>(T::typeid->Name) +
                "> '" + name + "' size=" + std::to_string(size));

            IntPtr ptr = Marshal::AllocHGlobal(size);
            T result;
            try
            {
                _native->ReadSymbol(name, ptr.ToPointer(), static_cast<size_t>(size));

                if (isBool)
                {
                    // Read single byte and convert to Boolean
                    unsigned char byteVal = *static_cast<unsigned char*>(ptr.ToPointer());
                    result = safe_cast<T>(static_cast<Boolean>(byteVal != 0));
                }
                else
                {
                    result = safe_cast<T>(Marshal::PtrToStructure(ptr, T::typeid));
                }
            }
            catch (...)
            {
                Marshal::FreeHGlobal(ptr);
                throw;
            }
            Marshal::FreeHGlobal(ptr);

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: ReadSymbol '" + name + "' completed successfully");

            return result;
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: ReadSymbol failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    generic <typename T> where T : value class
    void AdsDeviceWrapper::WriteSymbol(String^ symbolName, T value)
    {
        CheckDisposed();
        try
        {
            std::string name = marshal_as<std::string>(symbolName);

            // Marshal::SizeOf<bool> returns 4 (Win32 BOOL), but PLC BOOL is 1 byte
            bool isBool = (T::typeid == Boolean::typeid);
            int size = isBool ? 1 : Marshal::SizeOf(T::typeid);

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "AdsDeviceWrapper: WriteSymbol<" + marshal_as<std::string>(T::typeid->Name) +
                "> '" + name + "' size=" + std::to_string(size));

            IntPtr ptr = Marshal::AllocHGlobal(size);
            try
            {
                if (isBool)
                {
                    // Write single byte: 1 for true, 0 for false
                    unsigned char byteVal = safe_cast<Boolean>(value) ? 1 : 0;
                    *static_cast<unsigned char*>(ptr.ToPointer()) = byteVal;
                }
                else
                {
                    Marshal::StructureToPtr(value, ptr, false);
                }

                _native->WriteSymbol(name, ptr.ToPointer(), static_cast<size_t>(size));
            }
            catch (...)
            {
                Marshal::FreeHGlobal(ptr);
                throw;
            }
            Marshal::FreeHGlobal(ptr);

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: WriteSymbol '" + name + "' completed successfully");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: WriteSymbol failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }
}
