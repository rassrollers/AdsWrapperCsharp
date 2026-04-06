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
    ads.AddRemoteRoute(remoteName, remoteIp, rmNetId, remotePort, remoteUser, remotePassword);
    var state = ads.GetState();

    bool runLoop = true;
    while (runLoop)
    {
        Console.WriteLine("Type in command:\n" +
            "0: Exit\n" +
            "1: Reconfig\n" +
            "2: Reset\n" +
            "3: Read axis position\n" +
            "4: Power on axis\n" +
            "5: Move axis\n" +
            "6: Stop axis\n" +
            "7: Reset axis\n" +
            "8: Power off axis");

        var input = Console.ReadLine();
        switch (input)
        {
            case "0":
                runLoop = false;
                break;

            case "1":
                ads.SetTwinCatState(AdsState.Reconfig, 0);
                await Task.Delay(stateDelay);
                state = ads.GetState();
                break;

            case "2":
                ads.SetTwinCatState(AdsState.Reset, 0);
                await Task.Delay(stateDelay);
                state = ads.GetState();
                break;

            case "3":
                double position = ads.ReadSymbol<double>("MAIN.axisPosition");
                Console.WriteLine($"Axis position: {position}");
                break;

            case "4":
                ads.WriteSymbol<bool>("MAIN.powerOnAxis", true);
                Console.WriteLine("Axis powered on");
                break;

            case "5":
                ads.WriteSymbol<bool>("MAIN.moveAxis", true);
                Console.WriteLine("Axis move command sent");
                break;

            case "6":
                ads.WriteSymbol<bool>("MAIN.moveAxis", false);
                Console.WriteLine("Axis stop command sent");
                break;

            case "7":
                ads.WriteSymbol<bool>("MAIN.resetAxis", true);
                Console.WriteLine("Axis reset command sent");
                await Task.Delay(1000);
                ads.WriteSymbol<bool>("MAIN.resetAxis", false);
                break;

            case "8":
                ads.WriteSymbol<bool>("MAIN.powerOnAxis", false);
                Console.WriteLine("Axis powered off");
                break;

            default:
                Console.WriteLine("Invalid command");
                continue;
        }
    }
}
catch (Exception ex)
{
    Debug.WriteLine(ex);
}