#pragma once

using namespace System;

namespace AdsWrapper
{
    public enum class AmsPort : int
    {
        AdsRouter = 1,
        AmsDebugger = 2,
        TComServer = 10,
        TComServerTask = 11,
        TComServerPassive = 12,
        TwinCATDebugger = 20,
        TwinCATDebuggerTask = 21,
        LicenseServer = 30,
        // Logger
        Logger = 100,
        EventLogger = 110,
        EtherCATDeviceApplication = 120,
        EventLoggerUserMode = 130,
        EventLoggerRealtime = 131,
        EventLoggerPublisher = 132,
        // PLC System
        Ring0Realtime = 200,
        Ring0Trace = 290,
        Ring0IO = 300,
        Ring0PLCLegacy = 400,
        Ring0NC = 500,
        Ring0NCSEC = 501,
        Ring0NCSPP = 511,
        NCInstance = 520,
        RingISG = 550,
        Ring0CNC = 600,
        Ring0Line = 700,
        // TwinCAT 2
        Ring0TC2PLC = 800,
        TC2Runtime1 = 801,
        TC2Runtime2 = 811,
        TC2Runtime3 = 821,
        TC2Runtime4 = 831,
		// TwinCAT 3
        Ring0TC3PLC = 850,
        TC3Runtime1 = 851,
        TC3Runtime2 = 852,
        TC3Runtime3 = 853,
        TC3Runtime4 = 854,
        // CAM
        CamController = 900,
        CamTool = 950,
        Ring0IOPortsStart = 1000,
        Ring0IOPortsEnd = 1199,
        Ring0User = 2000,
        CrestronServer = 2500,
        // TwinCAT System
        SystemService = 10000,
        TcpIpServer = 10201,
        SystemManager = 10300,
        SmsServer = 10400,
        ModbusServer = 10500,
        AmsLogger = 10502,
        XmlDataServer = 10600,
        AutoConfiguration = 10700,
        PlcControl = 10800,
        FtpClient = 10900,
        NcControl = 11000,
        NcInterpreter = 11500,
        GstInterpreter = 11600,
        TrackControl = 12000,
        CamControl = 13000,
        ScopeServer = 14000,
        ConditionMonitoring = 14100,
        SineCH1 = 15000,
        ControlNet = 16000,
        OpcServer = 17000,
        OpcClient = 17500,
        MailServer = 18000,
        VirtualComEL60xx = 19000,
        ManagementServer = 19100,
        MieleHomeServer = 19200,
        CpLink3 = 19300,
        TouchLock = 19310,
        VisionService = 19500,
        HmiServer = 19800,
        DatabaseServer = 21372,
		// Reserved for ADS servers
        AdsServerRangeStart = 25000,
        AdsServerRangeEnd = 25999,
		// Third-party servers
        FiasServer = 25013,
        BangOlufsenServer = 25014,
		// Reserved for customer use
        CustomerPrivateStart = 26000,
        CustomerPrivateEnd = 26999,
		// Reserved for internal use by the client
        AdsClientReservedStart = 32768,
        AdsClientReservedEnd = 65535
    };

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