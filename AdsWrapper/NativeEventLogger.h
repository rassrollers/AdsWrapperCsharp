#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "AdsDevice.h"
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
    /// Parameters: the event header, decoded UTF-8 text, payload UniqueId, and SourceName.
    /// </summary>
    using EventCallback = std::function<void(const bhf::ads::TcEventEntry&, const std::string&, uint32_t, const std::string&)>;

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
    long Start();

    /// <summary>Unsubscribes from EventLogger notifications.</summary>
    void Stop();

private:
    static constexpr uint16_t AMSPORT_EVENTLOGGER_PUBLISHER_V2  = 132;
    // IndexGroup 0x309 = EventLogger Publisher V2 notification subscription.
    // IndexOffset encodes the minimum severity: 0x0000 = all (Verbose+).
    static constexpr uint32_t EVENTLOGGER_SUBSCRIBE_INDEXGROUP  = 0x00000309;
    static constexpr uint32_t EVENTLOGGER_SUBSCRIBE_INDEXOFFSET = 0x00000000;
    static constexpr uint32_t NOTIFICATION_BUFFER_SIZE          = 8192;

    // IndexGroup / IndexOffset for reading the EventLogger ring-buffer backlog.
    static constexpr uint32_t EVTLOG_INDEXGROUP  = 0x000000C8;
    static constexpr uint32_t EVTLOG_READ_INFO   = 0x00000001;
    static constexpr uint32_t EVTLOG_READ_EVENTS = 0x00000002;

    /// <summary>Reads buffered events from the EventLogger ring-buffer and fires the callback for each.</summary>
    void ReadBacklog();

    /// <summary>Parses a flat buffer of back-to-back TcEventEntry records and invokes the callback for each.</summary>
    void ParseEventBuffer(const uint8_t* data, uint32_t size);

    /// <summary>
    /// Parses the backlog ReadWrite response buffer, which uses a different container
    /// format from live notification TcEventEntry records.
    /// </summary>
    void ParseBacklogBuffer(const uint8_t* data, uint32_t size);

    /// <summary>
    /// Parses a single live-event notification record. Handles both direct
    /// GUID-based records and records prefixed by a 28-byte outer header.
    /// </summary>
    void ParseSingleEventRecord(const uint8_t* data, uint32_t size, uint64_t stampTimestamp);

    /// <summary>
    /// Parses one GUID-based EventLogger event block using fixed offsets.
    /// </summary>
    void ParseOffsetEventRecord(const uint8_t* block, uint32_t blockSize, uint64_t stampTimestamp);

    /// <summary>
    /// Static ADS notification callback.  Routes to the correct instance via
    /// the global registry using hUser as a lookup key.
    /// </summary>
    static void NotificationCallback(const AmsAddr*              pAddr,
                                     const AdsNotificationHeader* pNotification,
                                     uint32_t                     hUser);

    /// <summary>Parses and dispatches a single ADS notification payload.</summary>
    void HandleNotification(const AdsNotificationHeader* pNotification);

    AdsDevice                   m_Device;
    EventCallback               m_Callback;
    std::unique_ptr<AdsHandle>  m_NotificationHandle;  ///< non-null while subscribed
    uint32_t                    m_CallbackId;
    uint32_t                    m_SeverityFilter;      ///< IndexOffset passed to subscription

    // Registry mapping uint32_t IDs to NativeEventLogger instances so that
    // the static callback can safely find the owning instance.  hUser is
    // uint32_t in the ADS API, which is not wide enough to hold a pointer on
    // a 64-bit process – hence the indirection through this map.
    static std::mutex                                        s_RegistryMutex;
    static std::unordered_map<uint32_t, NativeEventLogger*> s_Registry;
    static std::atomic<uint32_t>                             s_NextId;
};
