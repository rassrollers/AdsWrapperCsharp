#include "pch.h"
#include "AdsDeviceWrapper.h"
#include "NativeAdsDevice.h"
#include "NativeLogger.h"

#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

namespace AdsWrapper
{
    AdsDeviceWrapper::AdsDeviceWrapper(String^ localIp,
        String^ remoteIp,
        UInt16 port,
        String^ routeName,
        String^ user,
        String^ password)
    {
        try
        {
            std::string lIp = marshal_as<std::string>(localIp);
            std::string rIp = marshal_as<std::string>(remoteIp);
            std::string rName = marshal_as<std::string>(routeName);
            std::string u = marshal_as<std::string>(user);
            std::string p = marshal_as<std::string>(password);

            _native = new NativeAdsDevice(lIp, rIp, port, rName, u, p);
        }
        catch (const std::exception& ex)
        {
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    AdsDeviceWrapper::~AdsDeviceWrapper()
    {
        this->!AdsDeviceWrapper();
        GC::SuppressFinalize(this);
    }

    AdsDeviceWrapper::!AdsDeviceWrapper()
    {
        delete _native;
        _native = nullptr;
    }

    AdsState AdsDeviceWrapper::GetState()
    {
        try
        {
            auto nativeState = _native->GetState();

            AdsState state;
            state.Ads = nativeState.ads;
            state.Device = nativeState.device;

            return state;
        }
        catch (const std::exception& ex)
        {
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }
}
