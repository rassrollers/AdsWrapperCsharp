using AdsWrapper;
using System.Diagnostics;

// Register a log callback before creating the device
LoggerWrapper.SetCallback((level, message) =>
{
    Console.WriteLine($"[{level}] {message}");
});

var localIp = "192.168.17.102";
var localNetId = "192.168.17.100.1.1";
var remoteIp = "192.168.17.10";
var remoteNetId = "192.168.17.10.1.1";
var remoteName = "C6015";
var remoteUser = "Administrator";
var remotePassword = "1";
var remotePort = AmsPort.SystemService;

var delay = TimeSpan.FromSeconds(3);

using var ads = new AdsDeviceWrapper(localIp, localNetId);

try
{
    ads.AddRemoteRoute(remoteName, remoteIp, remoteNetId, remotePort, remoteUser, remotePassword);
    var info = ads.GetDeviceInfo();
    Console.WriteLine($"Device info: {info.Name}, {info.Version}");
    var state = ads.GetState();
    Console.WriteLine($"Device state: {state.Ads}, {state.Device}");

    Console.WriteLine("Press Enter to set device to Reconfig state...");
    Console.ReadLine();
    ads.SetTwinCatState(AdsState.Reconfig, 0);

    await Task.Delay(delay);
    state = ads.GetState();
    Console.WriteLine($"Device state: {state.Ads}, {state.Device}");
    
    Console.WriteLine("Press Enter to set device to Reset state...");
    Console.ReadLine();
    ads.SetTwinCatState(AdsState.Reset, 0);

    await Task.Delay(delay);
    state = ads.GetState();
    Console.WriteLine($"Device state: {state.Ads}, {state.Device}");
}
catch (Exception ex)
{
    Debug.WriteLine(ex);
}