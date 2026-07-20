#include "pch.h"
#include "EventLoggerWrapper.h"
#include "NativeEventLogger.h"
#include "NativeLogger.h"

#include <vcclr.h>
#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

// ---------------------------------------------------------------------------
// Native callable struct used to forward ADS notifications to the managed
// wrapper.  Defined at file scope because C++/CLI does not allow lambda
// expressions (which are local class definitions) inside managed member
// functions.
// ---------------------------------------------------------------------------
struct EventLoggerNativeCallback
{
    gcroot<AdsWrapper::EventLoggerWrapper^>* pRoot;

    void operator()(const bhf::ads::TcEventEntry& entry,
                    const std::string&             text,
                    uint32_t                       uniqueId,
                    const std::string&             sourceName) const
    {
        (*pRoot)->OnNativeEvent(entry, text, uniqueId, sourceName);
    }
};

namespace AdsWrapper
{
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    EventLoggerWrapper::EventLoggerWrapper(String^ gateway, String^ netId,
                                           AmsPort port, System::UInt32 severityFilter)
        : _native(nullptr)
        , _disposed(false)
        , _pGcRoot(nullptr)
    {
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "EventLoggerWrapper: creating instance");

            std::string gw  = marshal_as<std::string>(gateway);
            std::string nid = marshal_as<std::string>(netId);
            uint16_t    p   = static_cast<uint16_t>(port);
            uint32_t    sf  = severityFilter;

            _native = new NativeEventLogger(gw, nid, p, sf);

            auto* pRoot = new gcroot<EventLoggerWrapper^>(this);
            _pGcRoot = pRoot;

            _native->SetCallback(EventLoggerNativeCallback{ pRoot });

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "EventLoggerWrapper: instance created");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "EventLoggerWrapper: construction failed – " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    EventLoggerWrapper::EventLoggerWrapper(String^ gateway, String^ netId, AmsPort port)
        : EventLoggerWrapper(gateway, netId, port, 0)
    {
    }

    EventLoggerWrapper::EventLoggerWrapper(String^ gateway, String^ netId)
        : EventLoggerWrapper(gateway, netId, AmsPort::EventLoggerPublisher, 0)
    {
    }

    // -------------------------------------------------------------------------
    // Disposal
    // -------------------------------------------------------------------------

    EventLoggerWrapper::~EventLoggerWrapper()
    {
        if (_disposed)
            return;

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
            "EventLoggerWrapper: disposing");
        this->!EventLoggerWrapper();
        GC::SuppressFinalize(this);
        _disposed = true;
    }

    EventLoggerWrapper::!EventLoggerWrapper()
    {
        if (_pGcRoot != nullptr)
        {
            delete static_cast<gcroot<EventLoggerWrapper^>*>(_pGcRoot);
            _pGcRoot = nullptr;
        }

        if (_native != nullptr)
        {
            delete _native;
            _native = nullptr;
        }
    }

    // -------------------------------------------------------------------------
    // CheckDisposed
    // -------------------------------------------------------------------------

    void EventLoggerWrapper::CheckDisposed()
    {
        if (_disposed)
            throw gcnew ObjectDisposedException("EventLoggerWrapper");
    }

    // -------------------------------------------------------------------------
    // Start
    // -------------------------------------------------------------------------

    long EventLoggerWrapper::Start()
    {
        CheckDisposed();
        try
        {
            long result = _native->Start();
            if (result == 0)
            {
                NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                    "EventLoggerWrapper: started");
            }
            else
            {
                NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                    "EventLoggerWrapper: Start() returned ADS error " +
                    std::to_string(result));
            }
            return result;
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "EventLoggerWrapper: Start() threw – " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    // -------------------------------------------------------------------------
    // Stop
    // -------------------------------------------------------------------------

    void EventLoggerWrapper::Stop()
    {
        if (_disposed || _native == nullptr)
            return;

        try
        {
            _native->Stop();
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Info,
                "EventLoggerWrapper: stopped");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "EventLoggerWrapper: Stop() threw – " + std::string(ex.what()));
        }
    }

    // -------------------------------------------------------------------------
    // OnNativeEvent – called from the ADS router thread via the native callback
    // -------------------------------------------------------------------------

    void EventLoggerWrapper::OnNativeEvent(const bhf::ads::TcEventEntry& entry,
                                           const std::string&             text,
                                           uint32_t                       uniqueId,
                                           const std::string&             sourceName)
    {
        TcEventArgs^ args = gcnew TcEventArgs();
        args->Severity  = static_cast<TcEventSeverity>(entry.nSeverity);
        args->EventType = static_cast<TcEventType>(entry.nEventType);
        args->SourceId  = entry.nSrcId;
        args->EventId   = entry.nEventId;
        args->UniqueId  = uniqueId;
        args->SourceName = gcnew String(sourceName.c_str());
        args->TimeStamp = DateTime::FromFileTimeUtc(static_cast<Int64>(entry.nTimeStamp));
        args->Text      = gcnew String(text.c_str());

        EventReceived(this, args);
    }
}
