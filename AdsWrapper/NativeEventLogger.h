#pragma once

#include <functional>
#include <memory>
#include <string>
#include "EventLoggerAccess.h"

/// <summary>
/// Native C++ class that subscribes to TwinCAT EventLogger Publisher V2 (AMS port 132)
/// and forwards each received event to a user-supplied callback.
/// </summary>
class NativeEventLogger
{
public:
    /// <summary>
    /// Callback invoked on the ADS router thread for every event received.
    /// Parameters: the event header, decoded UTF-8 text, and SourceName.
    /// </summary>
    using EventCallback = std::function<void(const bhf::ads::TcEventEntry&, const std::string&, const std::string&)>;

    /// <summary>
    /// Constructs an EventLogger connection.
    /// </summary>
    /// <param name="gw">IP address / hostname of the AMS router gateway</param>
    /// <param name="netId">AMS NetId of the target TwinCAT system (e.g. "192.168.0.1.1.1")</param>
    /// <param name="port">AMS port; 0 defaults to EventLogger Publisher V2 (132)</param>
    /// <param name="severityFilter">Minimum severity IndexOffset: 0=all, 1=Info+, 2=Warning+, 4=Error+, 8=Critical</param>
    NativeEventLogger(const std::string& gw, const std::string& netId,
                      uint16_t port = 0, uint32_t severityFilter = 0);

    /// <summary>Destructor. Automatically calls Stop().</summary>
    ~NativeEventLogger();

    NativeEventLogger(const NativeEventLogger&) = delete;
    NativeEventLogger& operator=(const NativeEventLogger&) = delete;

    /// <summary>Sets the callback invoked for each received event.</summary>
    void SetCallback(EventCallback callback);

    /// <summary>
    /// Subscribes to EventLogger notifications. Non-blocking.
    /// </summary>
    /// <returns>0 (ADSERR_NOERR) on success, or a non-zero ADS error code.</returns>
    long Subscribe();

    /// <summary>
    /// Reads and dispatches buffered events that accumulated before subscription.
    /// Call this after Subscribe() to retrieve the backlog.
    /// </summary>
    void ReadBacklog();

    /// <summary>Unsubscribes from EventLogger notifications.</summary>
    void Unsubscribe();

private:
    std::unique_ptr<bhf::ads::EventLoggerAccess> m_EventLogger;
};
