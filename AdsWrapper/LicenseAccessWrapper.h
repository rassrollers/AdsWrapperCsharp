#pragma once

#include <cstdint>

using namespace System;

class NativeLicenseAccess;  // Forward declaration

namespace AdsWrapper
{
    /// <summary>
    /// Managed wrapper class for reading TwinCAT license information.
    /// Provides .NET-friendly interface for accessing license details, system ID, and platform info.
    /// </summary>
    public ref class LicenseAccessWrapper : IDisposable
    {
    public:
        /// <summary>
        /// Initializes a new license access wrapper for a remote TwinCAT system.
        /// </summary>
        /// <param name="remoteIp">IP address of the remote TwinCAT system</param>
        /// <param name="remoteNetId">AMS NetId of the remote system (format: "x.x.x.x.x.x")</param>
        /// <param name="port">AMS port (default: 30)</param>
        LicenseAccessWrapper(String^ remoteIp, String^ remoteNetId, uint16_t port);

        /// <summary>
        /// Initializes a new license access wrapper with default port 30.
        /// </summary>
        LicenseAccessWrapper(String^ remoteIp, String^ remoteNetId);

        /// <summary>
        /// Managed destructor. Releases managed and unmanaged resources.
        /// </summary>
        ~LicenseAccessWrapper();

        /// <summary>
        /// Finalizer. Releases unmanaged resources.
        /// </summary>
        !LicenseAccessWrapper();

        /// <summary>
        /// Gets online license information as a formatted string (CSV format).
        /// </summary>
        /// <returns>License information with headers: OrderNo;Instances;Status;License</returns>
        /// <exception cref="Exception">Thrown when license info cannot be read</exception>
        String^ GetOnlineInfo();

        /// <summary>
        /// Gets the platform ID of the remote TwinCAT system.
        /// </summary>
        /// <returns>Platform ID</returns>
        /// <exception cref="Exception">Thrown when platform ID cannot be read</exception>
        uint16_t GetPlatformId();

        /// <summary>
        /// Gets the system ID (UUID) of the remote TwinCAT system.
        /// </summary>
        /// <returns>System ID in UUID format (e.g., "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")</returns>
        /// <exception cref="Exception">Thrown when system ID cannot be read</exception>
        String^ GetSystemId();

        /// <summary>
        /// Gets the volume number of the remote TwinCAT system.
        /// </summary>
        /// <returns>Volume number</returns>
        /// <exception cref="Exception">Thrown when volume number cannot be read</exception>
        uint32_t GetVolumeNo();

    private:
        NativeLicenseAccess* _native;
        bool _disposed = false;

        void CheckDisposed();
    };
}
