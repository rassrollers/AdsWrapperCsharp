using AdsWrapper;
using System.Diagnostics;

// Register a log callback before creating the device
LoggerWrapper.SetCallback((level, message) =>
{
    Console.WriteLine($"[{level}] {message}");
});

var localIp = "192.168.1.43";
var remoteIp = "192.168.1.10";
var remoteName = "C6015";
var remoteUser = "Administrator";
var remotePassword = "1";
ushort remotePort = 10000;

using var ads = new AdsDeviceWrapper(localIp);

try
{
    ads.AddRemoteRoute(remoteName, remoteIp, remotePort, remoteUser, remotePassword);
    var info = ads.GetDeviceInfo();
    Console.WriteLine($"Device info: {info.Name}, {info.Version}");
    var state = ads.GetState();
    Console.WriteLine($"Device state: {state.Ads}, {state.Device}");
    await Task.Delay(1000);
    ads.SetTwinCatState(AdsState.Reconfig, 0);

    await Task.Delay(5000);
    state = ads.GetState();
    Console.WriteLine($"Device state: {state.Ads}, {state.Device}");
    await Task.Delay(1000);
    ads.SetTwinCatState(AdsState.Reset, 0);

    await Task.Delay(5000);
    state = ads.GetState();
    Console.WriteLine($"Device state: {state.Ads}, {state.Device}");
}
catch (Exception ex)
{
    Debug.WriteLine(ex);
}