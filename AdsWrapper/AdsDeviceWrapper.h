#pragma once

using namespace System;
using namespace System::Threading::Tasks;

class NativeAdsDevice;   // Forward declaration

namespace AdsWrapper
{
    public value struct AdsState
    {
        int Ads;
        int Device;
    };

    public value struct DeviceInfo
    {
        String^ Name;
        int Version;
        int Revision;
        int Build;
	};

    public ref class AdsDeviceWrapper
    {
    public:

        AdsDeviceWrapper(String^ localIp);

        ~AdsDeviceWrapper();
        !AdsDeviceWrapper();

        void AddRemoteRoute(String^ routeName, 
            String^ remoteIp,
            UInt16 port,
            String^ user, 
            String^ password);

        AdsState GetState();

		DeviceInfo GetDeviceInfo();

    private:
        NativeAdsDevice* _native;
    };
}
