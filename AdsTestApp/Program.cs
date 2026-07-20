using AdsWrapper;
using System.Diagnostics;

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
var remotePort = AmsPort.TC3Runtime1;
var stateDelay = TimeSpan.FromSeconds(3);

try
{
    using var ads = new AdsDeviceWrapper(localIp, localNetId);
    var rmNetId = ads.GetRemoteNetId(remoteIp);
    Console.WriteLine($"Remote NetId for {remoteIp}: {rmNetId}");
    //Console.ReadLine();

    ads.AddRemoteRoute(remoteName, remoteIp, rmNetId, remotePort, remoteUser, remotePassword);

    using var logger = new AdsWrapper.EventLoggerWrapper(remoteIp, rmNetId);
    logger.EventReceived += (_, e) =>
        Console.WriteLine($"[{e.TimeStamp:u}] {e.Severity} SourceName={e.SourceName} UniqueId={e.UniqueId} {e.Text}");
    long err = logger.Start();   // subscribe
    Console.WriteLine("Press enter to stop");
    Console.ReadLine();
    logger.Stop();


    //var info = ads.GetDeviceInfo();
    //Console.WriteLine($"Device info: {info.Name}, {info.Version}");
    //var state = ads.GetState();
    //Console.WriteLine($"Device state: {state.Ads}, {state.Device}");

    //Console.WriteLine("Press Enter to set device to Reconfig state...");
    //Console.ReadLine();
    //ads.SetTwinCatState(AdsState.Reconfig, 0);

    //await Task.Delay(delay);
    //state = ads.GetState();
    //Console.WriteLine($"Device state: {state.Ads}, {state.Device}");
    
    //Console.WriteLine("Press Enter to set device to Reset state...");
    //Console.ReadLine();
    //ads.SetTwinCatState(AdsState.Reset, 0);

    //await Task.Delay(delay);
    //state = ads.GetState();
    //Console.WriteLine($"Device state: {state.Ads}, {state.Device}");
}
catch (Exception ex)
{
    Debug.WriteLine(ex);
}