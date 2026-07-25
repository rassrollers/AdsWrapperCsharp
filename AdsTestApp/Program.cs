using AdsWrapper;
using System.Diagnostics;
using AdsTestApp;

// Register a log callback before creating the device
LoggerWrapper.SetCallback((level, message) =>
{
    Console.WriteLine($"[{level}] {message}");
});

var localIp = "192.168.1.119";
var localNetId = "192.168.1.119.1.1";
var remoteIp = "192.168.1.10";
var remoteName = "C6015";
var remoteUser = "Administrator";
var remotePassword = "1";
var systemPort = AmsPort.SystemService;
var plcPort = AmsPort.TC3Runtime1;
var stateDelay = TimeSpan.FromSeconds(3);

try
{
    using var adsSystem = new AdsDeviceWrapper(localIp, localNetId);
    using var adsPlc = new AdsDeviceWrapper(localIp, localNetId);

    var rmNetId = adsSystem.GetRemoteNetId(remoteIp);
    Console.WriteLine($"Remote NetId for {remoteIp}: {rmNetId}");

    //adsSystem.AddRemoteRoute(remoteName, remoteIp, rmNetId, systemPort, remoteUser, remotePassword);
    
    adsPlc.AddRemoteRoute(remoteName, remoteIp, rmNetId, plcPort, remoteUser, remotePassword);

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
            Console.WriteLine($"Axis position: {position}");
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

    var mainMenu = new ConsoleMenu("ADS test menu")
        .AddOption("0", "Exit", _ => Task.CompletedTask, closeMenu: true)
        .AddOption("1", "adsSystem submenu", ct => systemMenu.RunAsync(ct))
        .AddOption("2", "adsPlc submenu", ct => plcMenu.RunAsync(ct));

    await mainMenu.RunAsync();
}
catch (Exception ex)
{
    Debug.WriteLine(ex);
}