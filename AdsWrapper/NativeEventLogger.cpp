#include "pch.h"
#include "NativeEventLogger.h"
#include "NativeLogger.h"
#include "AdsDef.h"

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
NativeEventLogger::NativeEventLogger(const std::string& gw,
                                     const std::string& netId,
                                     uint16_t           port,
                                     uint32_t           severityFilter)
{
    m_EventLogger = std::make_unique<bhf::ads::EventLoggerAccess>(
        gw,
        make_AmsNetId(netId),
        port,
        severityFilter);
}

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
NativeEventLogger::~NativeEventLogger()
{
    Unsubscribe();
}

// ---------------------------------------------------------------------------
// SetCallback
// ---------------------------------------------------------------------------
void NativeEventLogger::SetCallback(EventCallback callback)
{
    if (m_EventLogger) {
        m_EventLogger->SetCallback(std::move(callback));
    }
}

// ---------------------------------------------------------------------------
// Subscribe
// ---------------------------------------------------------------------------
long NativeEventLogger::Subscribe()
{
    if (!m_EventLogger) {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeEventLogger::Subscribe: EventLoggerAccess is not initialized");
        return ADSERR_CLIENT_SYNCRESINVALID;
    }

    const long result = m_EventLogger->Subscribe();
    if (result == ADSERR_NOERR) {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
            "NativeEventLogger: Subscribed to EventLogger");
    } else {
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
            "NativeEventLogger::Subscribe failed with ADS error " + std::to_string(result));
    }

    return result;
}

// ---------------------------------------------------------------------------
// ReadBacklog
// ---------------------------------------------------------------------------
void NativeEventLogger::ReadBacklog()
{
    if (m_EventLogger) {
        m_EventLogger->ReadBacklog();
    }
}

// ---------------------------------------------------------------------------
// Unsubscribe
// ---------------------------------------------------------------------------
void NativeEventLogger::Unsubscribe()
{
    if (m_EventLogger) {
        m_EventLogger->Unsubscribe();
        NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
            "NativeEventLogger: Unsubscribed from EventLogger");
    }
}
