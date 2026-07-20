using AdsWrapper;
using System.Diagnostics;
using AdsTestApp;
using System.Globalization;
using System.Text;

// Initialize logger with configuration from appsettings.json
LoggerSetup.Initialize();

var localIp = "192.168.1.119";
var localNetId = "192.168.1.119.1.1";
var remoteIp = "192.168.1.10";
var remoteName = "C6015";
var remoteUser = "Administrator";
var remotePassword = "1";
var stateDelay = TimeSpan.FromSeconds(3);
using var shutdownCts = new CancellationTokenSource();
EventLoggerWrapper? logger = null;
EventHandler<TcEventArgs>? eventLoggerHandler = null;
bool eventLoggerSubscribed = false;
var capturedEventRows = new List<string>();
const string eventCsvHeader =
    "TimeRaisedUtc,EventClassGuid,EventId,Severity,EventType,UniqueId,TimeClearedUtc,TimeConfirmedUtc,ConfirmationState,SourceId,SourceNameByteLength,DataByteLength,SourceName,Text";

static string EscapeCsv(string? value)
{
    if (string.IsNullOrEmpty(value))
    {
        return "\"\"";
    }

    var escaped = value.Replace("\"", "\"\"");
    return $"\"{escaped}\"";
}

static string BuildEventCsvRow(TcEventArgs e)
{
    var fields = new[]
    {
        e.EventClassGuid.ToString(),
        e.EventId.ToString(CultureInfo.InvariantCulture),
        e.Severity.ToString(),
        e.EventType.ToString(),
        e.UniqueId.ToString(CultureInfo.InvariantCulture),
        e.TimeRaised.ToUniversalTime().ToString("u", CultureInfo.InvariantCulture),
        e.TimeCleared.ToUniversalTime().ToString("u", CultureInfo.InvariantCulture),
        e.TimeConfirmed.ToUniversalTime().ToString("u", CultureInfo.InvariantCulture),
        e.ConfirmationState.ToString(),
        e.SourceId.ToString(CultureInfo.InvariantCulture),
        e.SourceNameByteLength.ToString(CultureInfo.InvariantCulture),
        e.DataByteLength.ToString(CultureInfo.InvariantCulture),
        e.SourceName ?? string.Empty,
        e.Text ?? string.Empty,
    };

    return string.Join(",", fields.Select(EscapeCsv));
}

ConsoleCancelEventHandler? cancelHandler = (_, e) =>
{
    e.Cancel = true;
    shutdownCts.Cancel();
    Console.WriteLine("Cancellation requested. Shutting down...");
};

Console.CancelKeyPress += cancelHandler;

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
    logger = new EventLoggerWrapper(remoteIp, rmNetId);

    eventLoggerHandler = (_, e) =>
    {
        lock (capturedEventRows)
        {
            capturedEventRows.Add(BuildEventCsvRow(e));
        }

        Console.WriteLine(
            $"EventClassGuid={e.EventClassGuid} " +
            $"EventId={e.EventId} " +
            $"Severity={e.Severity} " +
            $"EventType={e.EventType} " +
            $"UniqueId={e.UniqueId} " +
            $"TimeRaised={e.TimeRaised:u} " +
            $"TimeCleared={e.TimeCleared:u} " +
            $"TimeConfirmed={e.TimeConfirmed:u} " +
            $"ConfirmationState={e.ConfirmationState} " +
            $"SourceId={e.SourceId} " +
            $"SourceNameByteLength={e.SourceNameByteLength} " +
            $"DataByteLength={e.DataByteLength} " +
            $"SourceName={e.SourceName} " +
            $"Text={e.Text}");
    };

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

    var eventloggerMenu = new ConsoleMenu("Event Logger menu")
        .AddOption("0", "Back", _ => Task.CompletedTask, closeMenu: true)
        .AddOption("1", "Subscribe to EventLogger", _ =>
        {
            if (logger is null || eventLoggerHandler is null)
            {
                return Task.CompletedTask;
            }

            logger.EventReceived += eventLoggerHandler;
            var err = logger.Subscribe();
            if (err == 0)
            {
                eventLoggerSubscribed = true;
            }
            else
            {
                logger.EventReceived -= eventLoggerHandler;
            }

            return Task.CompletedTask;
        })
        .AddOption("2", "Read EventLogger backlog", _ =>
        {
            try
            {
                logger.ReadBacklog();
                Console.WriteLine("Backlog read completed");
            }
            catch (InvalidOperationException ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
            }
            return Task.CompletedTask;
        })
        .AddOption("3", "Unsubscribe from EventLogger", _ =>
        {
            if (logger is null || eventLoggerHandler is null)
            {
                return Task.CompletedTask;
            }

            logger.EventReceived -= eventLoggerHandler;
            logger.Unsubscribe();
            eventLoggerSubscribed = false;
            return Task.CompletedTask;
        })
        .AddOption("4", "Dump captured events to CSV", _ =>
        {
            List<string> rowsSnapshot;
            lock (capturedEventRows)
            {
                rowsSnapshot = new List<string>(capturedEventRows);
            }

            if (rowsSnapshot.Count == 0)
            {
                Console.WriteLine("No captured events to write.");
                return Task.CompletedTask;
            }

            var fileName = $"EventLoggerDump_{DateTime.UtcNow:yyyyMMdd_HHmmss}.csv";
            var filePath = Path.Combine(AppContext.BaseDirectory, fileName);

            using (var writer = new StreamWriter(filePath, false, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false)))
            {
                writer.WriteLine(eventCsvHeader);
                foreach (var row in rowsSnapshot)
                {
                    writer.WriteLine(row);
                }
            }

            Console.WriteLine($"Wrote {rowsSnapshot.Count} events to {filePath}");
            return Task.CompletedTask;
        })
        .AddOption("5", "Clear captured events", _ =>
        {
            int removedCount;
            lock (capturedEventRows)
            {
                removedCount = capturedEventRows.Count;
                capturedEventRows.Clear();
            }

            Console.WriteLine($"Cleared {removedCount} captured event(s).");
            return Task.CompletedTask;
        });

    var mainMenu = new ConsoleMenu("ADS test menu")
        .AddOption("0", "Exit", _ => Task.CompletedTask, closeMenu: true)
        .AddOption("1", "System Service menu", ct => systemMenu.RunAsync(ct))
        .AddOption("2", "PLC1 menu", ct => plcMenu.RunAsync(ct))
        .AddOption("3", "License menu", ct => licenseMenu.RunAsync(ct))
        .AddOption("4", "EventLogger menu", ct => eventloggerMenu.RunAsync(ct));

    await mainMenu.RunAsync(shutdownCts.Token);
}
catch (Exception ex)
{
    Debug.WriteLine(ex);
}
finally
{
    Console.CancelKeyPress -= cancelHandler;
    if (logger is not null && eventLoggerHandler is not null)
    {
        if (eventLoggerSubscribed)
        {
            logger.EventReceived -= eventLoggerHandler;
            logger.Unsubscribe();
        }

        logger.Dispose();
    }
}