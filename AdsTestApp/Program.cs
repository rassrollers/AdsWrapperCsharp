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
ushort remotePort = 851;

var ads = new AdsDeviceWrapper(localIp);

try
{
    ads.AddRemoteRoute(remoteName, remoteIp, remotePort, remoteUser, remotePassword);
    var info = ads.GetDeviceInfo();
    var state = ads.GetState();
}
catch (Exception ex)
{
    Debug.WriteLine(ex);
}