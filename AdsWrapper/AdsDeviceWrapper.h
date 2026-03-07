#pragma once

#include "AdsDefines.h"

using namespace System;
using namespace System::Threading::Tasks;

class NativeAdsDevice;   // Forward declaration

namespace AdsWrapper
{
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

		void SetTwinCatState(AdsState adsState, AdsState deviceState);
        StateInfo GetState();
		DeviceInfo GetDeviceInfo();

    private:
        NativeAdsDevice* _native;
    };
}
