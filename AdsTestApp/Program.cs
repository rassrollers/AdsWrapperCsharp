using AdsWrapper;
using System.Diagnostics;
using AdsTestApp;

// Initialize logger with configuration from appsettings.json
LoggerSetup.Initialize();

var localIp = "192.168.1.119";
var localNetId = "192.168.1.119.1.1";
var remoteIp = "192.168.1.10";
var remoteName = "C6015";
var remoteUser = "Administrator";
var remotePassword = "1";
var stateDelay = TimeSpan.FromSeconds(3);

try
{
    // Create the factory and configure the remote route
    using var adsFactory = new AdsDeviceWrapper(localIp, localNetId);
    var rmNetId = adsFactory.GetRemoteNetId(remoteIp);
    Console.WriteLine($"Remote NetID for IP {remoteIp}: {rmNetId}");
    adsFactory.AddRemoteRoute(remoteName, remoteIp, rmNetId, remoteUser, remotePassword);
    Console.WriteLine($"Remote route created for {rmNetId}");

    // Create individual device instances for system (10000) and PLC (851)
    using var adsSystem = adsFactory.CreateAdsDevice(AmsPort.SystemService);
    Console.WriteLine("Connection to AMS System Service port created");
    using var adsPlc = adsFactory.CreateAdsDevice(AmsPort.TC3Runtime1);
    Console.WriteLine("Connection to AMS TC3 PLC1 runtime port created");
    using var licenseAccess = new LicenseAccessWrapper(remoteIp, rmNetId);

    var systemMenu = new ConsoleMenu("ADS system menu")
        .AddOption("0", "Back", _ => Task.CompletedTask, closeMenu: true)
        .AddOption("1", "Read TwinCAT state", _ =>
        {
            var state = adsSystem.GetState();
            Console.WriteLine($"Current state: Ads={state.Ads}, Device={state.Device}");
            return Task.CompletedTask;
        })
        .AddOption("2", "Reconfig", async _ =>
        {
            adsSystem.SetTwinCatState(AdsState.Reconfig, 0);
            await Task.Delay(stateDelay);
            var state = adsSystem.GetState();
            Console.WriteLine($"Current state: Ads={state.Ads}, Device={state.Device}");
        })
        .AddOption("3", "Reset", async _ =>
        {
            adsSystem.SetTwinCatState(AdsState.Reset, 0);
            await Task.Delay(stateDelay);
            var state = adsSystem.GetState();
            Console.WriteLine($"Current state: Ads={state.Ads}, Device={state.Device}");
        });

    var plcMenu = new ConsoleMenu("ADS PLC menu")
        .AddOption("0", "Back", _ => Task.CompletedTask, closeMenu: true)
        .AddOption("1", "Power on axis", _ =>
        {
            adsPlc.WriteSymbol<bool>("MAIN.PowerOn", true);
            Console.WriteLine("Axis powered on");
            return Task.CompletedTask;
        })
        .AddOption("2", "Power off axis", _ =>
        {
            adsPlc.WriteSymbol<bool>("MAIN.PowerOn", false);
            Console.WriteLine("Axis powered off");
            return Task.CompletedTask;
        })
        .AddOption("3", "Reset axis", _ =>
        {
            adsPlc.WriteSymbol<bool>("MAIN.ResetAxis", true);
            Console.WriteLine("Axis reset command sent");
            return Task.CompletedTask;
        })
        .AddOption("4", "Read axis position", _ =>
        {
            var position = adsPlc.ReadSymbol<double>("MAIN.AxisPos");
            Console.WriteLine($"Axis position: {position:F5}m");
            return Task.CompletedTask;
        })
        .AddOption("5", "Read axis error ID", _ =>
        {
            var errorId = adsPlc.ReadSymbol<int>("MAIN.AxisErrorID");
            Console.WriteLine($"Axis error ID: 0x{errorId:X}");
            return Task.CompletedTask;
        })
        .AddOption("6", "Move axis", _ =>
        {
            adsPlc.WriteSymbol<bool>("MAIN.MoveAxis", true);
            Console.WriteLine("Axis move command sent");
            return Task.CompletedTask;
        })
        .AddOption("7", "Stop axis", _ =>
        {
            adsPlc.WriteSymbol<bool>("MAIN.StopAxis", true);
            Console.WriteLine("Axis stop command sent");
            return Task.CompletedTask;
        });

    var licenseMenu = new ConsoleMenu("License Info menu")
        .AddOption("0", "Back", _ => Task.CompletedTask, closeMenu: true)
        .AddOption("1", "Show online licenses", _ =>
        {
            var onlineInfo = licenseAccess.GetOnlineInfo();
            if (string.IsNullOrEmpty(onlineInfo))
                return Task.CompletedTask;
            Console.WriteLine("Online license info:");
            Console.WriteLine(onlineInfo);
            return Task.CompletedTask;
        })
        .AddOption("2", "Show platform ID", _ =>
        {
            var platformId = licenseAccess.GetPlatformId();
            Console.WriteLine($"Platform ID: {platformId}");
            return Task.CompletedTask;
        })
        .AddOption("3", "Show system ID", _ =>
        {
            var systemId = licenseAccess.GetSystemId();
            Console.WriteLine($"System ID: {systemId}");
            return Task.CompletedTask;
        })
        .AddOption("4", "Show volume number", _ =>
        {
            var volumeNo = licenseAccess.GetVolumeNo();
            Console.WriteLine($"Volume number: {volumeNo}");
            return Task.CompletedTask;
        });

    var mainMenu = new ConsoleMenu("ADS test menu")
        .AddOption("0", "Exit", _ => Task.CompletedTask, closeMenu: true)
        .AddOption("1", "System Service menu", ct => systemMenu.RunAsync(ct))
        .AddOption("2", "PLC1 menu", ct => plcMenu.RunAsync(ct))
        .AddOption("3", "License menu", ct => licenseMenu.RunAsync(ct));

    await mainMenu.RunAsync();
}
catch (Exception ex)
{
    Debug.WriteLine(ex);
}