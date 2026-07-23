#include "pch.h"
#include "EtcDeviceWrapper.h"
#include "NativeEtcDevice.h"
#include "NativeLogger.h"

#include <msclr/marshal_cppstd.h>

// Suppress false IntelliSense errors in C++/CLI generic methods
#ifdef __INTELLISENSE__
#define generic
#define where
#endif
#include <sstream>

using namespace msclr::interop;

namespace AdsWrapper
{
    EtcDeviceWrapper::EtcDeviceWrapper(String^ remoteIp, String^ remoteNetId)
    {
        CheckDisposed();
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: Setting up EtherCAT master access");
            std::string rIp = marshal_as<std::string>(remoteIp);
            std::string rNetId = marshal_as<std::string>(remoteNetId);

			_native = new NativeEtcDevice(rIp, rNetId);
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: SetupEtcMaster completed successfully");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: SetupEtcMaster failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    EtcDeviceWrapper::~EtcDeviceWrapper()
    {
        if (_disposed)
            return;

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
            "AdsDeviceWrapper: Disposing");
        this->!EtcDeviceWrapper();
        GC::SuppressFinalize(this);
        _disposed = true;
    }

    EtcDeviceWrapper::!EtcDeviceWrapper()
    {
        if (_native != nullptr)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: Releasing native resources");
            delete _native;
            _native = nullptr;
        }
    }

    void EtcDeviceWrapper::CheckDisposed()
    {
        if (_disposed)
            throw gcnew ObjectDisposedException("AdsDeviceWrapper");
    }

    String^ EtcDeviceWrapper::ListAllEtcMasters()
    {
        CheckDisposed();
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: Listing all EtherCAT masters");
            std::ostringstream oss;
            long status = _native->ListAllEtcMasters(oss);
            if (status)
            {
                NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                    "AdsDeviceWrapper: ListAllEtcMasters failed with status " + std::to_string(status));
                throw gcnew Exception("ListAllEtcMasters failed with status " + status.ToString());
            }

            std::string result = oss.str();
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: ListAllEtcMasters completed successfully");
            return gcnew String(result.c_str());
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: ListAllEtcMasters failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    array<ECatMasterSlaveStatus>^ EtcDeviceWrapper::GetECatSlaveAlStatus()
    {
        CheckDisposed();
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: Getting EtherCAT slave AL status");
            auto nativeResult = _native->GetECatSlaveAlStatus();
            if (nativeResult.empty())
            {
                NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                    "AdsDeviceWrapper: No EtherCAT slave status found");
                return gcnew array<ECatMasterSlaveStatus>(0);
            }

            auto managedResult = gcnew array<ECatMasterSlaveStatus>(static_cast<int>(nativeResult.size()));

            int index = 0;
            for (const auto& pair : nativeResult)
            {
                const AmsNetId& netId = pair.first;

                ECatMasterSlaveStatus entry;
                entry.MasterNetId = gcnew String((
                    std::to_string(netId.b[0]) + "." +
                    std::to_string(netId.b[1]) + "." +
                    std::to_string(netId.b[2]) + "." +
                    std::to_string(netId.b[3]) + "." +
                    std::to_string(netId.b[4]) + "." +
                    std::to_string(netId.b[5])).c_str());

                const auto& statuses = pair.second;
                entry.SlaveAlStatuses = gcnew array<unsigned short>(static_cast<int>(statuses.size()));
                if (statuses.empty())
                {
                    NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                        "AdsDeviceWrapper: No slave AL status found for master " +
                        marshal_as<std::string>(entry.MasterNetId));
                }
                for (int i = 0; i < static_cast<int>(statuses.size()); ++i)
                {
                    entry.SlaveAlStatuses[i] = statuses[i];
                }

                managedResult[index++] = entry;
            }

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "AdsDeviceWrapper: GetECatSlaveAlStatus completed successfully, found " +
                std::to_string(nativeResult.size()) + " master(s)");

            return managedResult;
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "AdsDeviceWrapper: GetECatSlaveAlStatus failed - " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }
}