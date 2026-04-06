#pragma once

#include <memory>
#include <string>
#include <vector>
#include "AdsLib.h"
#include "AdsDevice.h"

/// <summary>
/// Native C++ wrapper class for managing ADS (Automation Device Specification) device connections.
/// Handles AMS routing, device state management, and communication with TwinCAT systems.
/// </summary>
class NativeAdsDevice
{
public:
    /// <summary>
    /// Constructs a new ADS device connection with local network configuration.
    /// </summary>
    /// <param name="localIp">The local IP address for AMS communication</param>
    /// <param name="localNetId">The local AMS NetId (format: "x.x.x.x.x.x")</param>
    NativeAdsDevice(const std::string& localIp, const std::string& localNetId);

    /// <summary>
    /// Destructor. Cleans up ADS device resources.
    /// </summary>
    ~NativeAdsDevice();
    
    // Prevent copying
    NativeAdsDevice(const NativeAdsDevice&) = delete;
    NativeAdsDevice& operator=(const NativeAdsDevice&) = delete;
    
    /// <summary>
    /// Adds a remote AMS route and establishes connection to a remote ADS device.
    /// </summary>
    /// <param name="routeName">Friendly name for the route</param>
    /// <param name="remoteIp">IP address of the remote TwinCAT system</param>
    /// <param name="remoteNetId">AMS NetId of the remote system (format: "x.x.x.x.x.x")</param>
    /// <param name="amsPort">AMS port number (e.g., 851 for PLC runtime)</param>
    /// <param name="user">Username for authentication (if required)</param>
    /// <param name="password">Password for authentication (if required)</param>
    /// <exception cref="AdsException">Thrown when route creation fails</exception>
    void AddRemoteRoute(const std::string& routeName,
        const std::string& remoteIp,
        const std::string& remoteNetId,
        uint16_t amsPort,
        const std::string& user,
        const std::string& password);

    /// <summary>
    /// Retrieves the AMS NetId for a remote system by its IP address.
    /// </summary>
    /// <param name="remoteIp">IP address of the remote system</param>
    /// <param name="netId">Output parameter that receives the NetId string (format: "x.x.x.x.x.x")</param>
    /// <exception cref="AdsException">Thrown when NetId retrieval fails</exception>
    void GetRemoteNetId(const std::string& remoteIp, std::string& netId);

    /// <summary>
    /// Sets the TwinCAT system state.
    /// </summary>
    /// <param name="adsState">The desired ADS state (e.g., CONFIG, RUN, STOP)</param>
    /// <param name="deviceState">Device-specific state value</param>
    /// <exception cref="std::runtime_error">Thrown if device not initialized</exception>
    void SetTwinCatState(ADSSTATE adsState, ADSSTATE deviceState);

    /// <summary>
    /// Gets the current state of the ADS device.
    /// </summary>
    /// <returns>AdsDeviceState containing current ADS state and device state</returns>
    /// <exception cref="std::runtime_error">Thrown if device not initialized</exception>
    AdsDeviceState GetState() const;

    /// <summary>
    /// Gets information about the connected ADS device.
    /// </summary>
    /// <returns>DeviceInfo containing device name, version, and other metadata</returns>
    /// <exception cref="std::runtime_error">Thrown if device not initialized</exception>
    DeviceInfo GetDeviceInfo() const;

    /// <summary>
    /// Reads a PLC symbol by name into a raw byte buffer.
    /// Uses AdsVariable-style handle-based access (ADSIGRP_SYM_VALBYHND).
    /// </summary>
    /// <param name="symbolName">Fully qualified PLC symbol name (e.g., "MAIN.myVar")</param>
    /// <param name="buffer">Output buffer to receive the symbol data</param>
    /// <param name="bufferSize">Size in bytes of the expected data</param>
    /// <exception cref="std::runtime_error">Thrown if device not initialized</exception>
    /// <exception cref="AdsException">Thrown on ADS communication error or size mismatch</exception>
    void ReadSymbol(const std::string& symbolName, void* buffer, size_t bufferSize) const;

    /// <summary>
    /// Writes a raw byte buffer to a PLC symbol by name.
    /// Uses AdsVariable-style handle-based access (ADSIGRP_SYM_VALBYHND).
    /// </summary>
    /// <param name="symbolName">Fully qualified PLC symbol name (e.g., "MAIN.myVar")</param>
    /// <param name="buffer">Data buffer to write</param>
    /// <param name="bufferSize">Size in bytes of the data</param>
    /// <exception cref="std::runtime_error">Thrown if device not initialized</exception>
    /// <exception cref="AdsException">Thrown on ADS communication error</exception>
    void WriteSymbol(const std::string& symbolName, const void* buffer, size_t bufferSize) const;

private:
    std::unique_ptr<AmsNetId> _localAms;    ///< Local AMS NetId
    std::unique_ptr<AmsNetId> _remoteAms;   ///< Remote AMS NetId
    std::unique_ptr<AdsDevice> _device;     ///< The underlying ADS device connection

    std::string _localIp;   ///< Local IP address
    std::string _remoteIp;  ///< Remote IP address
};
