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

    public ref class AdsDeviceWrapper
    {
    public:

        AdsDeviceWrapper(String^ localIp,
            String^ remoteIp,
            UInt16 port,
            String^ routeName,
            String^ user,
            String^ password);

        ~AdsDeviceWrapper();
        !AdsDeviceWrapper();

        AdsState GetState();

    private:
        NativeAdsDevice* _native;
    };
}
