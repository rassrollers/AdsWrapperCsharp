# ADS wrapper for C# on Windows

This is a wrapper for the ADS library for C# projects on Windows.

ADS library is based on the [Beckhoff ADS repo](https://github.com/Beckhoff/ADS)


## Setup solution

### ADS library

* Make a Static C++ library project.
* Add the header and source files from the ADS GIT repo.
* Select all .cpp files and disable Precompiled Header.
    * C/C++ -> Precompiled Headers -> Precompiled Header = `Not Using Precompiled Headers`
* Add additional include path to the ADS library file in the project properties.
    * C/C++ -> General -> Additional Include Directories = `$(SolutionDir)ADS\AdsLib`
* Changes the output files directory to not get error on duplicated files (standalone/TwinCAT directories).
    * C/C++ -> Output Files -> Object File Name = `$(IntDir)%(RelativeDir)`
* Add the following to the Preprocessor Defines:
    * `CONFIG_DEFAULT_LOGLEVEL=1`
    * `_CRT_SECURE_NO_WARNINGS`
    * `NOMINMAX`
* Exclude the header and source files from the AdsLib/TwinCAT directory for building an standalone library.
    * Require you to have TwinCAT installed. 

### ADS wrapper project

* Add projet reference to the AdsLib project.
* Setup the C++/CLI properties
    * Advanced -> Common Language Runtime Support = `.NET Runtime Support (/clr:netcore)`
    * Advanced -> .NET Target Windows Version = `7.0`
* Add additional include path to the ADS library file in the project properties.
    * C/C++ -> General -> Additional Include Directories = `$(SolutionDir)ADS\AdsLib`

Add the following code to the top of your header to fix the winsock issues:
```C++
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
```

### C# project

* Add projet reference to the AdsLib project.
* Setup platform settings to match wrapper project.
    * General -> Target OS = `Windows`
    * General -> Target OS version = `7.0`
