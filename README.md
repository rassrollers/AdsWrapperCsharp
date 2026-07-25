# ADS wrapper for C# on Windows

This is a C# wrapper for the Beckhoff ADS library on Windows.

It is a standalone ADS client that can be used on Windows PC without anye TwinCAT installation on it.

ADS library is based on the [Beckhoff ADS repo](https://github.com/Beckhoff/ADS)

**Note: If you use this standalone ADS client on PC with TwinCAT installed, it might clashes with the ADS router**

## Functionality

The wrapper have the following functionalities:

* Add a route to a remote target.
* Get remote AMS Net ID.
* Get the ADS state.
* Set the ADS state.
* Get the device info.
* Read/Write symbol by name.
* Access license information (platform ID, system ID, volume number, online license info).

## Quick Start

### Step 1: Clone the Repository with Submodules

Clone this repository and initialize the ADS submodule:

```shell
git clone https://github.com/rassrollers/AdsWrapperCsharp.git
cd AdsWrapperCsharp
git submodule init
git submodule update
```

### Step 2: Create Your C# Project and Add References

* Create a new C# Console App project in the same solution.
* Add project references to both `AdsLib` and `AdsWrapper`:
    * Right-click on your C# project → Add → Project Reference → Select both `AdsLib` and `AdsWrapper`
* Setup platform settings to match the wrapper project:
    * Project Properties → General → Target OS = `Windows`
    * Project Properties → General → Target OS version = `7.0`

### Step 3: Start Using the Wrapper

Here's a basic example of how to use the ADS wrapper in your C# project:

```csharp
using AdsWrapper;
using AdsTestApp;

// Initialize logger with configuration from appsettings.json
LoggerSetup.Initialize();

var localIp = "192.168.1.119";
var localNetId = "192.168.1.119.1.1";
var remoteIp = "192.168.1.10";
var remoteName = "C6015";
var remoteUser = "Administrator";
var remotePassword = "1";

try
{
    // Create the factory and configure the remote route (once)
    using var adsFactory = new AdsDeviceWrapper(localIp, localNetId);
    var remoteNetId = adsFactory.GetRemoteNetId(remoteIp);
    adsFactory.AddRemoteRoute(remoteName, remoteIp, remoteNetId, remoteUser, remotePassword);

    // Create individual device instances for system and PLC (can create multiple)
    using var adsSystem = adsFactory.CreateAdsDevice(AmsPort.SystemService);
    using var adsPlc = adsFactory.CreateAdsDevice(AmsPort.TC3Runtime1);

    // Use adsSystem for system operations
    var state = adsSystem.GetState();
    Console.WriteLine($"System ADS state: {state}");

    // Use adsPlc for PLC operations
    var info = adsPlc.GetDeviceInfo();
    Console.WriteLine($"PLC device info: {info}");

    Console.WriteLine("Set PLC in run");
    adsSystem.SetTwinCatState(AdsState.Run, 0);

    double position = adsPlc.ReadSymbol<double>("MAIN.axisPosition");
    Console.WriteLine($"Axis position: {position}");

    Console.WriteLine("Axis powered on");
    adsPlc.WriteSymbol<bool>("MAIN.powerOnAxis", true);

    // Access license information
    using var licenseAccess = new LicenseAccessWrapper(remoteIp, remoteNetId);
    var onlineInfo = licenseAccess.GetOnlineInfo();
    Console.WriteLine($"License info:\n{onlineInfo}");
    
    var platformId = licenseAccess.GetPlatformId();
    Console.WriteLine($"Platform ID: 0x{platformId:X4}");
    
    var systemId = licenseAccess.GetSystemId();
    Console.WriteLine($"System ID: {systemId}");
    
    var volumeNo = licenseAccess.GetVolumeNo();
    Console.WriteLine($"Volume number: {volumeNo}");
}
catch (Exception ex)
{
    Console.WriteLine($"Error: {ex.Message}");
}
```

### Multiple ADS Device Factory Pattern

The wrapper uses a factory pattern for managing multiple ADS devices:

1. **Create a Factory**: Instantiate one `AdsDeviceWrapper` as the factory with local network settings
2. **Configure Route Once**: Call `AddRemoteRoute()` to set up the remote connection
3. **Create Multiple Devices**: Call `CreateAdsDevice(port)` multiple times with different AMS ports to create independent device instances
4. **Independent Cleanup**: Each wrapper instance manages its own resources and is cleaned up via `using` statements

This approach allows you to:
- Manage multiple ADS connections (e.g., SystemService and PLC runtime)
- Avoid repeated route setup for multiple devices
- Keep resource management clean and deterministic

## Getting the solution to build

Getting the static and dynamic library to build in Visual Studio.

### ADS library

* Make a C++ Static Library project.
* Add the header and source files from the ADS GIT repo.
* Select all .cpp files and disable Precompiled Header.
    * C/C++ -> Precompiled Headers -> Precompiled Header = `Not Using Precompiled Headers`
* Add additional include path to the ADS library file in the project properties.
    * C/C++ -> General -> Additional Include Directories = `$(ProjectDir)..\ADS\AdsLib`
* Changes the output files directory to not get error on duplicated files (standalone/TwinCAT directories).
    * C/C++ -> Output Files -> Object File Name = `$(IntDir)%(RelativeDir)`
* Add the following to the Preprocessor Defines:
    * `CONFIG_DEFAULT_LOGLEVEL=1`
    * `_CRT_SECURE_NO_WARNINGS`
    * `NOMINMAX`
* Exclude the header and source files from the AdsLib/TwinCAT directory for building an standalone library.
    * Require you to have TwinCAT installed. 

### ADS wrapper project

* Make a C++ Dynamic-Link Library (DLL) project.
* Add projet reference to the AdsLib project.
* Setup the C++/CLI properties
    * Advanced -> Common Language Runtime Support = `.NET Runtime Support (/clr:netcore)`
    * Advanced -> .NET Target Windows Version = `7.0`
* Add additional include path to the ADS library file in the project properties.
    * C/C++ -> General -> Additional Include Directories = `$(ProjectDir)..\ADS\AdsLib`

Add the following code to the pch.h header to fix the winsock issues:
```C++
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
```

### C# test project

* Make a C# Console App.
* Add projet reference to the AdsLib project.
* Setup platform settings to match wrapper project.
    * General -> Target OS = `Windows`
    * General -> Target OS version = `7.0`
