#pragma once

#include "AdsDefines.h"

using namespace System;
using namespace System::Threading::Tasks;

class NativeAdsDevice;   // Forward declaration

namespace AdsWrapper
{
    public ref class AdsDeviceWrapper : IDisposable
    {
    public:

        AdsDeviceWrapper(String^ localIp, String^ localNetId);

        ~AdsDeviceWrapper();
        !AdsDeviceWrapper();
		void CheckDisposed();

        void AddRemoteRoute(String^ routeName, 
            String^ remoteIp,
			String^ remoteNetId,
            AmsPort amsPort,
            String^ user, 
            String^ password);

		void SetTwinCatState(AdsState adsState, AdsState deviceState);
        StateInfo GetState();
		DeviceInfo GetDeviceInfo();
    private:
        NativeAdsDevice* _native;
		bool _disposed = false;
    };
}
