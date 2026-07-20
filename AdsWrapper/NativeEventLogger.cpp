#include "pch.h"
#include "NativeEventLogger.h"
#include "NativeLogger.h"
#include "AdsException.h"
#include "AdsDef.h"

#include <cstdint>
#include <cstring>
#include <vector>

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------
std::mutex                                        NativeEventLogger::s_RegistryMutex;
std::unordered_map<uint32_t, NativeEventLogger*> NativeEventLogger::s_Registry;
std::atomic<uint32_t>                             NativeEventLogger::s_NextId{ 1 };

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
NativeEventLogger::NativeEventLogger(const std::string& gw,
                                     const std::string& netId,
                                     uint16_t           port,
                                     uint32_t           severityFilter)
    : m_Device(gw, make_AmsNetId(netId), port ? port : AMSPORT_EVENTLOGGER_PUBLISHER_V2)
    , m_CallbackId(0)
    , m_SeverityFilter(severityFilter)
{
}

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
NativeEventLogger::~NativeEventLogger()
{
    Stop();
}

// ---------------------------------------------------------------------------
// SetCallback
// ---------------------------------------------------------------------------
void NativeEventLogger::SetCallback(EventCallback callback)
{
    m_Callback = std::move(callback);
}

// ---------------------------------------------------------------------------
// Start
// ---------------------------------------------------------------------------
long NativeEventLogger::Start()
{
    // Allocate a unique ID for this instance (used as hUser in the ADS callback).
    m_CallbackId = s_NextId.fetch_add(1);
    {
        std::lock_guard<std::mutex> lock(s_RegistryMutex);
        s_Registry[m_CallbackId] = this;
    }

    const AdsNotificationAttrib attrib = {
        8192,                     // nCbLength – buffer size for EventLogger notifications
        ADSTRANS_SERVERCYCLE,     // nTransMode
        0,                        // nMaxDelay
        { 0 }                     // dwChangeFilter / nCycleTime
    };

    try {
        m_NotificationHandle = std::make_unique<AdsHandle>(
            m_Device.GetHandle(
                EVENTLOGGER_SUBSCRIBE_INDEXGROUP,
                m_SeverityFilter,
                attrib,
                reinterpret_cast<PAdsNotificationFuncEx>(&NativeEventLogger::NotificationCallback),
                m_CallbackId));

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
            "NativeEventLogger: subscribed to EventLogger on AMS port " +
            std::to_string(m_Device.m_Addr.port));

        // Deliver events buffered in the EventLogger ring-buffer before
        // we subscribed so no history is silently lost.
        ReadBacklog();

        return ADSERR_NOERR;
    }
    catch (const AdsException& ex) {
        {
            std::lock_guard<std::mutex> lock(s_RegistryMutex);
            s_Registry.erase(m_CallbackId);
        }
        m_CallbackId = 0;
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeEventLogger::Start() failed with ADS error " +
            std::to_string(ex.errorCode));
        return ex.errorCode;
    }
}

// ---------------------------------------------------------------------------
// Stop
// ---------------------------------------------------------------------------
void NativeEventLogger::Stop()
{
    // Destroying the AdsHandle sends the ADS delete-notification request.
    m_NotificationHandle.reset();

    if (m_CallbackId != 0) {
        std::lock_guard<std::mutex> lock(s_RegistryMutex);
        s_Registry.erase(m_CallbackId);
        m_CallbackId = 0;
    }

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
        "NativeEventLogger: unsubscribed from EventLogger");
}

// ---------------------------------------------------------------------------
// NotificationCallback (static) – routes to the owning instance
// ---------------------------------------------------------------------------
void NativeEventLogger::NotificationCallback(const AmsAddr*               /*pAddr*/,
                                             const AdsNotificationHeader* pNotification,
                                             uint32_t                     hUser)
{
    if (!pNotification) {
        return;
    }

    NativeEventLogger* self = nullptr;
    {
        std::lock_guard<std::mutex> lock(s_RegistryMutex);
        auto it = s_Registry.find(hUser);
        if (it == s_Registry.end()) {
            return;
        }
        self = it->second;
    }

    self->HandleNotification(pNotification);
}

// ---------------------------------------------------------------------------
// HandleNotification – dispatches a single ADS notification payload
// ---------------------------------------------------------------------------
void NativeEventLogger::HandleNotification(const AdsNotificationHeader* pNotification)
{
    if (!m_Callback) return;

    const uint8_t* raw  = reinterpret_cast<const uint8_t*>(pNotification + 1);
    const uint32_t size = pNotification->cbSampleSize;

    // 16-byte packets are heartbeat ticks – no event data.
    if (size <= 16) return;

    // Live event notifications use the same extended event record format as
    // the backlog (32-byte fixed header + variable payload).  Parse as a
    // single record; the ADS notification stamp provides a reliable timestamp.
    ParseSingleEventRecord(raw, size, pNotification->nTimeStamp);
}

// ---------------------------------------------------------------------------
// ReadBacklog – reads events that accumulated before we subscribed
// ---------------------------------------------------------------------------
void NativeEventLogger::ReadBacklog()
{
    // The ReadWrite(0xC8, 0x02) expects a fixed 16-byte header structure
    // that specifies the read operation parameters. Do NOT use a cursor from 0xC8/0x01.
    // Format (little-endian):
    //   - version (0x01)
    //   - IndexGroup (0xC8)
    //   - unknown field (0x04)
    //   - buffer size hint (1000)
    struct BacklogHeader {
        uint32_t version;
        uint32_t indexGroup;
        uint32_t unknown;
        uint32_t bufferSize;
    } header;
    
    header.version = 0x01;
    header.indexGroup = EVTLOG_INDEXGROUP;  // 0xC8
    header.unknown = 0x04;
    header.bufferSize = 1000;

    std::vector<uint8_t> eventBuf(0x100000);
    uint32_t eventBytes = 0;
    
    const long readErr = m_Device.ReadWriteReqEx2(
        EVTLOG_INDEXGROUP, EVTLOG_READ_EVENTS,
        eventBuf.size(), eventBuf.data(),
        sizeof(header), reinterpret_cast<uint8_t*>(&header),
        &eventBytes);

    if (readErr) {
        char hexBuf[16];
        snprintf(hexBuf, sizeof(hexBuf), "%x", static_cast<unsigned int>(readErr));
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Warning,
            "ReadBacklog: ReadWrite(0xC8, 0x02) failed with ADS error " +
            std::to_string(readErr) + " (0x" + std::string(hexBuf) + ")");
        return;
    }

    if (eventBytes == 0) {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
            "ReadBacklog: Backlog empty (0 bytes received)");
        return;
    }

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
        "ReadBacklog: Delivering " + std::to_string(eventBytes) +
        " bytes of backlogged events");

    ParseBacklogBuffer(eventBuf.data(), eventBytes);
}

// ---------------------------------------------------------------------------
// FormatEventText – substitutes printf-style specifiers in an event template.
//
// Parameters are packed immediately after the template's null terminator:
//   %s  → null-terminated ASCII string
//   %d  → 4-byte little-endian int32_t
//   %u  → 4-byte little-endian uint32_t
//   %x  → 4-byte little-endian uint32_t (hex)
// ---------------------------------------------------------------------------
static std::string FormatEventText(const std::string& tmpl,
                                   const uint8_t*     params,
                                   size_t             paramSize)
{
    if (params == nullptr || paramSize == 0 || tmpl.find('%') == std::string::npos) {
        return tmpl;
    }

    std::string      result;
    const uint8_t*   p   = params;
    const uint8_t*   end = params + paramSize;
    result.reserve(tmpl.size() + 64);

    for (size_t i = 0; i < tmpl.size(); ++i) {
        if (tmpl[i] != '%' || i + 1 >= tmpl.size()) {
            result += tmpl[i];
            continue;
        }

        const char spec = tmpl[i + 1];
        ++i; // consume the specifier character

        if (spec == 's') {
            // Read null-terminated string parameter.
            while (p < end && *p != 0) {
                result += static_cast<char>(*p++);
            }
            if (p < end) ++p; // skip null terminator
        } else if (spec == 'd' || spec == 'i') {
            if (p + 4 <= end) {
                int32_t val = 0;
                memcpy(&val, p, 4);
                p += 4;
                result += std::to_string(val);
            }
        } else if (spec == 'u') {
            if (p + 4 <= end) {
                uint32_t val = 0;
                memcpy(&val, p, 4);
                p += 4;
                result += std::to_string(val);
            }
        } else if (spec == 'x' || spec == 'X') {
            if (p + 4 <= end) {
                uint32_t val = 0;
                memcpy(&val, p, 4);
                p += 4;
                char buf[16];
                snprintf(buf, sizeof(buf), (spec == 'X') ? "%X" : "%x", val);
                result += buf;
            }
        } else {
            // Unknown specifier – emit literally.
            result += '%';
            result += spec;
        }
    }
    return result;
}

static std::string ReadAsciiCString(const uint8_t* start, const uint8_t* end)
{
    if (start == nullptr || end == nullptr || start >= end) {
        return std::string();
    }

    const uint8_t* p = start;
    while (p < end && *p != 0) {
        ++p;
    }

    std::string result;
    result.reserve(static_cast<size_t>(p - start));
    for (const uint8_t* c = start; c < p; ++c) {
        const uint8_t ch = *c;
        result += static_cast<char>((ch >= 0x20 && ch < 0x7F) ? ch : '?');
    }
    return result;
}

static std::string ExtractBestPrintableString(const uint8_t* start,
                                              const uint8_t* end,
                                              const uint8_t** outAfterNull)
{
    if (outAfterNull) {
        *outAfterNull = nullptr;
    }

    if (start == nullptr || end == nullptr || start >= end) {
        return std::string();
    }

    std::string best;
    const uint8_t* bestAfterNull = nullptr;
    bool bestHasFormat = false;

    for (const uint8_t* p = start; p < end; ) {
        while (p < end && (*p < 0x20 || *p >= 0x7F)) {
            ++p;
        }
        if (p >= end) {
            break;
        }

        const uint8_t* strStart = p;
        while (p < end && *p >= 0x20 && *p < 0x7F) {
            ++p;
        }

        const size_t len = static_cast<size_t>(p - strStart);
        if (len >= 4) {
            std::string candidate(reinterpret_cast<const char*>(strStart), len);
            const bool hasFormat = candidate.find('%') != std::string::npos;

            if ((hasFormat && !bestHasFormat) ||
                (hasFormat == bestHasFormat && len > best.size())) {
                best = std::move(candidate);
                bestHasFormat = hasFormat;
                bestAfterNull = (p < end && *p == 0) ? (p + 1) : p;
            }
        }

        if (p < end && *p == 0) {
            ++p;
        }
    }

    if (outAfterNull) {
        *outAfterNull = bestAfterNull;
    }

    return best;
}

static bool LooksLikeOffsetEventRecord(const uint8_t* block, uint32_t blockSize)
{
    if (block == nullptr || blockSize < 0x98) {
        return false;
    }

    static constexpr uint32_t OFF_SEVERITY     = 0x18;
    static constexpr uint32_t OFF_SOURCE_NAME  = 0x94;
    static constexpr uint16_t MARKER_DIRECT    = 0x000E;
    static constexpr uint16_t MARKER_GAP48     = 0x0014;

    uint16_t severity = 0;
    memcpy(&severity, block + OFF_SEVERITY, sizeof(severity));
    if (severity > 16) {
        return false;
    }

    const uint8_t* end = block + blockSize;
    const uint8_t* p = block + OFF_SOURCE_NAME;
    if (p >= end) {
        return false;
    }

    size_t sourceLen = 0;
    while (p + sourceLen < end && p[sourceLen] != 0) {
        const uint8_t ch = p[sourceLen];
        if (ch < 0x20 || ch >= 0x7F) {
            return false;
        }
        ++sourceLen;
        if (sourceLen > 64) {
            return false;
        }
    }

    if (sourceLen == 0 || p + sourceLen >= end) {
        return false;
    }

    const uint8_t* afterSource = p + sourceLen + 1;
    if (afterSource + sizeof(uint16_t) > end) {
        return false;
    }

    // Control word is usually right after SourceName, but some payload variants
    // include small padding/metadata before it.
    const uint8_t* searchEnd = afterSource + 64;
    if (searchEnd > end) {
        searchEnd = end;
    }

    for (const uint8_t* q = afterSource; q + sizeof(uint16_t) <= searchEnd; ++q) {
        uint16_t marker = 0;
        memcpy(&marker, q, sizeof(marker));
        if (marker == MARKER_DIRECT || marker == MARKER_GAP48) {
            return true;
        }
    }

    return false;
}

static const uint8_t* FindOffsetEventRecordStart(const uint8_t* raw, uint32_t size)
{
    if (raw == nullptr || size < 0x98) {
        return nullptr;
    }

    const uint8_t* end = raw + size;

    // Fast paths for known placements.
    if (LooksLikeOffsetEventRecord(raw, size)) {
        return raw;
    }
    if (size > 28 && LooksLikeOffsetEventRecord(raw + 28, size - 28)) {
        return raw + 28;
    }

    // Some live packets can include additional framing/network bytes.
    // Find the first slice that looks like a valid offset-based record.
    for (const uint8_t* p = raw; p + 0x98 <= end; ++p) {
        if (LooksLikeOffsetEventRecord(p, static_cast<uint32_t>(end - p))) {
            return p;
        }
    }

    return nullptr;
}

static bool TryGetOffsetEventBlockLength(const uint8_t* block,
                                         uint32_t       availableBytes,
                                         uint32_t&      outBlockLength)
{
    if (block == nullptr || availableBytes < 0x74) {
        return false;
    }

    uint32_t payloadOffset = 0;
    uint32_t payloadLength = 0;
    memcpy(&payloadOffset, block + 0x6C, sizeof(payloadOffset));
    memcpy(&payloadLength, block + 0x70, sizeof(payloadLength));

    // Observed layout in mixed 0x000E/0x0014 backlog data:
    // event block size = payloadOffset + payloadLength.
    const uint64_t total = static_cast<uint64_t>(payloadOffset) +
                           static_cast<uint64_t>(payloadLength);

    if (payloadOffset < 0x90 || payloadLength == 0 || total > availableBytes || total > 0x01000000ULL) {
        return false;
    }

    outBlockLength = static_cast<uint32_t>(total);
    return outBlockLength >= 0x98;
}

static const uint8_t* FindNextGuidMarker(const uint8_t* start,
                                         const uint8_t* end,
                                         const uint8_t* guid16)
{
    if (start == nullptr || end == nullptr || guid16 == nullptr || start >= end) {
        return nullptr;
    }

    for (const uint8_t* p = start; p + 16 <= end; ++p) {
        if (memcmp(p, guid16, 16) == 0) {
            return p;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// ParseOffsetEventRecord – parses one GUID-based event record using
// fixed offsets observed in the EventLogger payload.
// ---------------------------------------------------------------------------
void NativeEventLogger::ParseOffsetEventRecord(const uint8_t* block,
                                               uint32_t       blockSize,
                                               uint64_t       stampTimestamp)
{
    if (!m_Callback || block == nullptr || blockSize < 0x98) {
        return;
    }

    // Offsets relative to start of GUID-based event block.
    static constexpr uint32_t OFF_EVENT_ID           = 0x10;
    static constexpr uint32_t OFF_EVENT_FLAGS        = 0x14;
    static constexpr uint32_t OFF_SEVERITY           = 0x18;
    static constexpr uint32_t OFF_UNIQUE_ID          = 0x2C;
    static constexpr uint32_t OFF_TIME_RAISED        = 0x30;
    static constexpr uint32_t OFF_TIME_CLEARED       = 0x38;
    static constexpr uint32_t OFF_CONFIRM_STATE      = 0x48;
    static constexpr uint32_t OFF_TIME_CONFIRMED     = 0x50;
    static constexpr uint32_t OFF_SOURCE_ID          = 0x90;
    static constexpr uint32_t OFF_SOURCE_NAME        = 0x94;
    static constexpr uint16_t CUSTOM_TEXT_MARKER_DIRECT = 0x000E;
    static constexpr uint16_t CUSTOM_TEXT_MARKER_GAP48  = 0x0014;
    static constexpr uint32_t CUSTOM_TEXT_GAP48_BYTES   = 48;

    if (blockSize <= OFF_SOURCE_NAME) {
        return;
    }

    uint16_t severity       = 0;
    uint32_t eventId        = 0;
    uint32_t eventFlags     = 0;
    uint32_t uniqueId       = 0;
    uint32_t sourceId       = 0;
    uint16_t confirmState   = 0;
    uint64_t timeRaised     = 0;
    uint64_t timeCleared    = 0;
    uint64_t timeConfirmed  = 0;

    memcpy(&eventId,       block + OFF_EVENT_ID,       sizeof(eventId));
    memcpy(&eventFlags,    block + OFF_EVENT_FLAGS,    sizeof(eventFlags));
    memcpy(&severity,      block + OFF_SEVERITY,       sizeof(severity));
    memcpy(&uniqueId,      block + OFF_UNIQUE_ID,      sizeof(uniqueId));
    memcpy(&timeRaised,    block + OFF_TIME_RAISED,    sizeof(timeRaised));
    memcpy(&timeCleared,   block + OFF_TIME_CLEARED,   sizeof(timeCleared));
    memcpy(&confirmState,  block + OFF_CONFIRM_STATE,  sizeof(confirmState));
    memcpy(&timeConfirmed, block + OFF_TIME_CONFIRMED, sizeof(timeConfirmed));
    memcpy(&sourceId,      block + OFF_SOURCE_ID,      sizeof(sourceId));

    // Fall back to ADS notification timestamp when the payload FILETIME is missing/invalid.
    uint64_t eventTime = timeRaised;
    static constexpr uint64_t FILETIME_YEAR2000 = 0x01BF53EB211D0000ULL;
    if (eventTime < FILETIME_YEAR2000) {
        eventTime = stampTimestamp;
    }

    const uint8_t* blockEnd = block + blockSize;
    const uint8_t* sourceNameStart = block + OFF_SOURCE_NAME;
    std::string sourceName = ReadAsciiCString(sourceNameStart, blockEnd);

    const uint8_t* afterSourceName = sourceNameStart;
    while (afterSourceName < blockEnd && *afterSourceName != 0) {
        ++afterSourceName;
    }
    if (afterSourceName < blockEnd) {
        ++afterSourceName;
    }

    const uint8_t* customTextStart = nullptr;
    const uint8_t* markerPos = nullptr;
    if (afterSourceName + sizeof(uint16_t) <= blockEnd) {
        const uint8_t* markerSearchEnd = afterSourceName + 64;
        if (markerSearchEnd > blockEnd) {
            markerSearchEnd = blockEnd;
        }

        uint16_t controlWord = 0;
        for (const uint8_t* p = afterSourceName; p + sizeof(uint16_t) <= markerSearchEnd; ++p) {
            memcpy(&controlWord, p, sizeof(controlWord));
            if (controlWord == CUSTOM_TEXT_MARKER_DIRECT || controlWord == CUSTOM_TEXT_MARKER_GAP48) {
                markerPos = p;
                break;
            }
        }

        if (markerPos != nullptr) {
            memcpy(&controlWord, markerPos, sizeof(controlWord));
            if (controlWord == CUSTOM_TEXT_MARKER_DIRECT) {
                customTextStart = markerPos + sizeof(uint16_t);
            } else if (controlWord == CUSTOM_TEXT_MARKER_GAP48) {
                const uint8_t* candidate = markerPos + sizeof(uint16_t) + CUSTOM_TEXT_GAP48_BYTES;
                while (candidate < blockEnd && *candidate == 0) {
                    ++candidate;
                }
                if (candidate < blockEnd) {
                    customTextStart = candidate;
                }
            }
        }
    }

    std::string text;
    const uint8_t* paramStart = nullptr;
    if (customTextStart != nullptr && customTextStart < blockEnd) {
        text = ReadAsciiCString(customTextStart, blockEnd);
        if (!text.empty()) {
            paramStart = customTextStart + text.size();
            if (paramStart < blockEnd && *paramStart == 0) {
                ++paramStart;
            }
        }

        // 0x0014 variants can contain metadata/sub-structures before the actual
        // template text. Prefer a scanned printable/template string there.
        if (markerPos != nullptr) {
            uint16_t controlWord = 0;
            memcpy(&controlWord, markerPos, sizeof(controlWord));
            if (controlWord == CUSTOM_TEXT_MARKER_GAP48) {
                const uint8_t* scannedParamStart = nullptr;
                const std::string scanned = ExtractBestPrintableString(customTextStart, blockEnd, &scannedParamStart);
                if (!scanned.empty()) {
                    text = scanned;
                    paramStart = scannedParamStart;
                }
            }
        }
    }

    // Fallback: if marker was absent or empty, extract the longest printable string after SourceName.
    if (text.empty() && afterSourceName < blockEnd) {
        for (const uint8_t* p = afterSourceName; p < blockEnd; ) {
            while (p < blockEnd && (*p < 0x20 || *p >= 0x7F)) {
                ++p;
            }
            if (p >= blockEnd) {
                break;
            }

            const uint8_t* runStart = p;
            while (p < blockEnd && *p >= 0x20 && *p < 0x7F) {
                ++p;
            }

            const size_t len = static_cast<size_t>(p - runStart);
            if (len > text.size()) {
                text.assign(reinterpret_cast<const char*>(runStart), len);
            }

            if (p < blockEnd && *p == 0) {
                ++p;
            }
        }
    }

    if (text.find('%') != std::string::npos && paramStart != nullptr && paramStart < blockEnd) {
            text = FormatEventText(text, paramStart, static_cast<size_t>(blockEnd - paramStart));
    }

    bhf::ads::TcEventEntry entry{};
    entry.nSeverity  = severity;
    entry.nEventType = static_cast<uint16_t>((eventFlags >> 16) & 0xFFFF);
    entry.nSrcId     = sourceId;
    entry.nEventId   = eventId;
    entry.nTimeStamp = eventTime;
    entry.nCbData    = 0;
    const bhf::ads::TcEventEntry callbackEntry = entry;
    const std::string            callbackText = text;
    const uint32_t               callbackUniqueId = uniqueId;
    const std::string            callbackSourceName = sourceName;
    m_Callback(callbackEntry, callbackText, callbackUniqueId, callbackSourceName);

    (void)timeCleared;
    (void)confirmState;
    (void)timeConfirmed;
}

// ---------------------------------------------------------------------------
// ParseSingleEventRecord – parses one live-event notification
// ---------------------------------------------------------------------------
void NativeEventLogger::ParseSingleEventRecord(const uint8_t* raw, uint32_t size,
                                               uint64_t       stampTimestamp)
{
    if (!m_Callback || raw == nullptr || size < 16) {
        return;
    }

    const uint8_t* start = FindOffsetEventRecordStart(raw, size);
    if (start != nullptr) {
        ParseOffsetEventRecord(start, static_cast<uint32_t>((raw + size) - start), stampTimestamp);
    } else if (size >= 0x98) {
        // Retain compatibility fallback for unknown variants.
        ParseOffsetEventRecord(raw, size, stampTimestamp);
    }
}

// ---------------------------------------------------------------------------
// ParseBacklogBuffer – parses the ReadWrite(0xC8, 0x02) response.
// Container begins with a 28-byte header; event blocks begin with GUID.
// ---------------------------------------------------------------------------
void NativeEventLogger::ParseBacklogBuffer(const uint8_t* raw, uint32_t totalBytes)
{
    if (!m_Callback || totalBytes < 52) {
        return;
    }

    // Read nEvents from outer header at offset 16.
    uint32_t nEvents = 0;
    memcpy(&nEvents, raw + 16, sizeof(nEvents));

    if (nEvents == 0 || nEvents > 10000) {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Warning,
            "ParseBacklogBuffer: invalid nEvents = " + std::to_string(nEvents));
        return;
    }

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
        "ParseBacklogBuffer: parsing " + std::to_string(nEvents) + " backlog events");

    const uint8_t* end       = raw + totalBytes;
    const uint8_t* blockGuid = raw + 28;

    // First event block starts at the first GUID right after the 28-byte header.
    const uint8_t* blockStart = raw + 28;

    uint32_t parsedBlocks = 0;
    while (blockStart + 16 <= end)
    {
        const uint32_t available = static_cast<uint32_t>(end - blockStart);
        uint32_t blockSize = 0;

        if (!TryGetOffsetEventBlockLength(blockStart, available, blockSize)) {
            // Fallback path: split by next repeated GUID marker.
            const uint8_t* nextGuid = FindNextGuidMarker(blockStart + 16, end, blockGuid);
            if (nextGuid != nullptr) {
                blockSize = static_cast<uint32_t>(nextGuid - blockStart);
                NativeLogger::Instance().Log(NativeLogger::LogLevel::Warning,
                    "ParseBacklogBuffer: fallback GUID-boundary block size at index " +
                    std::to_string(parsedBlocks) + " (" + std::to_string(blockSize) + " bytes)");
            } else {
                blockSize = available;
                NativeLogger::Instance().Log(NativeLogger::LogLevel::Warning,
                    "ParseBacklogBuffer: fallback to tail block at index " +
                    std::to_string(parsedBlocks) + " (" + std::to_string(blockSize) + " bytes)");
            }
        }

        if (blockSize < 16) {
            break;
        }

        if (LooksLikeOffsetEventRecord(blockStart, blockSize)) {
            ParseOffsetEventRecord(blockStart, blockSize, 0);
        } else {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Warning,
                "ParseBacklogBuffer: skipped non-matching block at index " +
                std::to_string(parsedBlocks) + " (" + std::to_string(blockSize) + " bytes)");
        }

        ++parsedBlocks;
        if (parsedBlocks > 100000) {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Warning,
                "ParseBacklogBuffer: safety break after excessive block count");
            break;
        }

        if (blockStart + blockSize >= end) {
            break;
        }
        blockStart += blockSize;
    }
}

// ---------------------------------------------------------------------------
// ParseEventBuffer – parses flat TcEventEntry buffer and fires the callback
// ---------------------------------------------------------------------------
void NativeEventLogger::ParseEventBuffer(const uint8_t* raw, uint32_t remaining)
{
    if (!m_Callback) {
        return;
    }

    while (remaining >= sizeof(bhf::ads::TcEventEntry)) {
        const bhf::ads::TcEventEntry* entry =
            reinterpret_cast<const bhf::ads::TcEventEntry*>(raw);

        const uint32_t totalSize =
            static_cast<uint32_t>(sizeof(bhf::ads::TcEventEntry)) + entry->nCbData;

        if (totalSize > remaining) {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Warning,
                "NativeEventLogger: truncated event entry, stopping parse");
            break;
        }

        // Decode the UTF-16LE payload to a narrow string.
        std::string text;
        if (entry->nCbData >= sizeof(uint16_t)) {
            const uint16_t* wchars =
                reinterpret_cast<const uint16_t*>(raw + sizeof(bhf::ads::TcEventEntry));
            const size_t numWchars = entry->nCbData / sizeof(uint16_t);
            text.reserve(numWchars);
            for (size_t i = 0; i < numWchars; ++i) {
                const uint16_t wc = wchars[i];
                if (wc == 0) break;
                text += static_cast<char>((wc >= 0x20 && wc < 0x7F) ? wc : '?');
            }
        }

        const bhf::ads::TcEventEntry callbackEntry = *entry;
        const std::string            callbackText = text;
        const uint32_t               callbackUniqueId = 0;
        const std::string            callbackSourceName;
        m_Callback(callbackEntry, callbackText, callbackUniqueId, callbackSourceName);

        raw       += totalSize;
        remaining -= totalSize;
    }
}
