#pragma once

#include <memory>
#include <string>
#include "LicenseAccess.h"

class NativeLicenseAccess
{
public:
    /// <summary>
    /// Constructs a license access wrapper for reading license information.
    /// </summary>
    /// <param name="remoteIp">The gateway/remote IP address</param>
    /// <param name="remoteNetId">The remote AMS NetId</param>
    /// <param name="port">The AMS port (default: 30)</param>
    NativeLicenseAccess(const std::string& remoteIp, const std::string& remoteNetId, uint16_t port = 30);

    ~NativeLicenseAccess();

    // Prevent copying
    NativeLicenseAccess(const NativeLicenseAccess&) = delete;
    NativeLicenseAccess& operator=(const NativeLicenseAccess&) = delete;

    /// <summary>
    /// Gets online license information as a formatted string.
    /// </summary>
    /// <returns>CSV-formatted license information</returns>
    /// <exception cref="std::runtime_error">Thrown if license info cannot be read</exception>
    std::string GetOnlineInfo() const;

    /// <summary>
    /// Gets the platform ID.
    /// </summary>
    /// <returns>Platform ID as uint16_t</returns>
    /// <exception cref="std::runtime_error">Thrown if platform ID cannot be read</exception>
    uint16_t GetPlatformId() const;

    /// <summary>
    /// Gets the system ID (UUID format).
    /// </summary>
    /// <returns>System ID as formatted string (UUID)</returns>
    /// <exception cref="std::runtime_error">Thrown if system ID cannot be read</exception>
    std::string GetSystemId() const;

    /// <summary>
    /// Gets the volume number.
    /// </summary>
    /// <returns>Volume number as uint32_t</returns>
    /// <exception cref="std::runtime_error">Thrown if volume number cannot be read</exception>
    uint32_t GetVolumeNo() const;

private:
    std::unique_ptr<bhf::ads::LicenseAccess> _license;
};
