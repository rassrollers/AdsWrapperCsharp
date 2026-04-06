#pragma once

#include "AdsDefines.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Threading::Tasks;

class NativeAdsDevice;   // Forward declaration

namespace AdsWrapper
{
    /// <summary>
    /// Managed wrapper class for ADS (Automation Device Specification) device connections.
    /// Provides .NET-friendly interface for communicating with Beckhoff TwinCAT systems.
    /// Implements IDisposable for proper resource cleanup.
    /// </summary>
    public ref class AdsDeviceWrapper : IDisposable
    {
    public:
        /// <summary>
        /// Initializes a new ADS device connection with local network configuration.
        /// </summary>
        /// <param name="localIp">The local IP address for AMS communication</param>
        /// <param name="localNetId">The local AMS NetId (format: "x.x.x.x.x.x")</param>
        AdsDeviceWrapper(String^ localIp, String^ localNetId);

        /// <summary>
        /// Managed destructor. Releases managed and unmanaged resources.
        /// </summary>
        ~AdsDeviceWrapper();

        /// <summary>
        /// Finalizer. Releases unmanaged resources.
        /// </summary>
        !AdsDeviceWrapper();

        /// <summary>
        /// Checks if the object has been disposed and throws if true.
        /// </summary>
        /// <exception cref="ObjectDisposedException">Thrown if object is already disposed</exception>
        void CheckDisposed();

        /// <summary>
        /// Adds a remote AMS route and establishes connection to a remote ADS device.
        /// </summary>
        /// <param name="routeName">Friendly name for the route</param>
        /// <param name="remoteIp">IP address of the remote TwinCAT system</param>
        /// <param name="remoteNetId">AMS NetId of the remote system (format: "x.x.x.x.x.x")</param>
        /// <param name="amsPort">AMS port number (e.g., PlcRuntime1 = 851)</param>
        /// <param name="user">Username for authentication (empty string if not required)</param>
        /// <param name="password">Password for authentication (empty string if not required)</param>
        /// <exception cref="Exception">Thrown when route creation fails</exception>
        void AddRemoteRoute(String^ routeName, 
            String^ remoteIp,
            String^ remoteNetId,
            AmsPort amsPort,
            String^ user, 
            String^ password);

        /// <summary>
        /// Retrieves the AMS NetId for a remote system by its IP address.
        /// Useful for discovering the NetId of a TwinCAT system when only the IP is known.
        /// </summary>
        /// <param name="remoteIp">IP address of the remote system</param>
        /// <returns>The AMS NetId string (format: "x.x.x.x.x.x")</returns>
        /// <exception cref="Exception">Thrown when NetId retrieval fails</exception>
        String^ GetRemoteNetId(String^ remoteIp);

        /// <summary>
        /// Sets the TwinCAT system state (e.g., CONFIG, RUN, STOP).
        /// </summary>
        /// <param name="adsState">The desired ADS state</param>
        /// <param name="deviceState">Device-specific state value (typically same as adsState)</param>
        /// <exception cref="ObjectDisposedException">Thrown if object is disposed</exception>
        void SetTwinCatState(AdsState adsState, AdsState deviceState);

        /// <summary>
        /// Gets the current state of the ADS device.
        /// </summary>
        /// <returns>StateInfo containing current ADS state and device state</returns>
        /// <exception cref="ObjectDisposedException">Thrown if object is disposed</exception>
        StateInfo GetState();

        /// <summary>
        /// Gets information about the connected ADS device.
        /// </summary>
        /// <returns>DeviceInfo containing device name, version, and other metadata</returns>
        /// <exception cref="ObjectDisposedException">Thrown if object is disposed</exception>
        DeviceInfo GetDeviceInfo();

        /// <summary>
        /// Reads a PLC symbol by name and returns its value as the specified unmanaged type.
        /// Supported types: bool, byte, short, int, long, float, double, etc.
        /// </summary>
        /// <typeparam name="T">An unmanaged value type matching the PLC symbol's data type</typeparam>
        /// <param name="symbolName">Fully qualified PLC symbol name (e.g., "MAIN.myVar")</param>
        /// <returns>The current value of the PLC symbol</returns>
        /// <exception cref="ObjectDisposedException">Thrown if object is disposed</exception>
        generic <typename T> where T : value class
        T ReadSymbol(String^ symbolName);

        /// <summary>
        /// Writes a value to a PLC symbol by name.
        /// Supported types: bool, byte, short, int, long, float, double, etc.
        /// </summary>
        /// <typeparam name="T">An unmanaged value type matching the PLC symbol's data type</typeparam>
        /// <param name="symbolName">Fully qualified PLC symbol name (e.g., "MAIN.myVar")</param>
        /// <param name="value">The value to write</param>
        /// <exception cref="ObjectDisposedException">Thrown if object is disposed</exception>
        /// <exception cref="Exception">Thrown on ADS communication error</exception>
        generic <typename T> where T : value class
        void WriteSymbol(String^ symbolName, T value);

    private:
        NativeAdsDevice* _native;   ///< Pointer to native C++ ADS device implementation
        bool _disposed = false;     ///< Flag indicating if object has been disposed
    };
}
