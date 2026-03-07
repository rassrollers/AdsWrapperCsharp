#pragma once

using namespace System;

namespace AdsWrapper
{
    public enum class AdsState : int
    {
        Invalid = 0,
        Idle = 1,
        Reset = 2,        // Used for setting system in RUN mode
        Init = 3,
        Start = 4,
        Run = 5,
        Stop = 6,
        SaveConfig = 7,
        LoadConfig = 8,
        PowerFailure = 9,
        PowerGood = 10,
        Error = 11,
        Shutdown = 12,
        Suspend = 13,
        Resume = 14,
        Config = 15,
        Reconfig = 16,    // Used for setting system in CONFIG mode
        Stopping = 17,
        Incompatible = 18,
        Exception = 19
    };

    public value struct StateInfo
    {
        AdsState Ads;
        AdsState Device;
    };

    public value struct DeviceInfo
    {
        String^ Name;
        int Version;
        int Revision;
        int Build;
    };
}