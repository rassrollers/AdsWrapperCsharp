using AdsWrapper;
using System.Diagnostics;

// Register a log callback before creating the device
LoggerWrapper.SetCallback((level, message) =>
{
    Console.WriteLine($"[{level}] {message}");
});

var localIp = "192.168.1.43";
var localNetId = "192.168.1.43.1.1";
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

    ads.AddRemoteRoute(remoteName, remoteIp, rmNetId, remotePort, remoteUser, remotePassword);
    var state = ads.GetState();
    Console.WriteLine($"Device state: {state.Ads}, {state.Device}");

    // EtherCAT master discovery
    using var etc = new EtcDeviceWrapper(remoteIp, rmNetId);

    var masters = etc.ListAllEtcMasters();
    Console.WriteLine("EtherCAT Masters:");
    Console.WriteLine(masters);

    var slaveStatus = etc.GetECatSlaveAlStatus();
    if (slaveStatus == null || slaveStatus.Length == 0)
    {
        Console.WriteLine("No EtherCAT slave status was found.");
    }
    else
    {
        foreach (var master in slaveStatus)
        {
            Console.WriteLine($"Master: {master.MasterNetId}");
            if (master.SlaveAlStatuses == null || master.SlaveAlStatuses.Length == 0)
            {
                Console.WriteLine("  No slave AL status found for this master.");
                continue;
            }
            foreach (var alStatus in master.SlaveAlStatuses)
            {
                Console.WriteLine($"  Slave AL Status: 0x{alStatus:X4}");
            }
        }
    }
}
catch (Exception ex)
{
    Debug.WriteLine(ex);
}