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
                    const std::string&             sourceName) const
    {
        (*pRoot)->OnNativeEvent(entry, text, sourceName);
    }
};

static System::DateTime ConvertFileTimeOrFallback(uint64_t fileTime, System::DateTime fallback)
{
    if (fileTime == 0) {
        return fallback;
    }

    try
    {
        return System::DateTime::FromFileTimeUtc(static_cast<System::Int64>(fileTime));
    }
    catch (System::ArgumentOutOfRangeException^)
    {
        return fallback;
    }
}

namespace AdsWrapper
{
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    EventLoggerWrapper::EventLoggerWrapper(String^ gateway, String^ netId,
                                           AmsPort port, System::UInt32 severityFilter)
        : _native(nullptr)
        , _disposed(false)
        , _subscribed(false)
        , _pGcRoot(nullptr)
    {
        try
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "EventLoggerWrapper: Creating new instance");

            std::string gw  = marshal_as<std::string>(gateway);
            std::string nid = marshal_as<std::string>(netId);
            uint16_t    p   = static_cast<uint16_t>(port);
            uint32_t    sf  = severityFilter;

            _native = new NativeEventLogger(gw, nid, p, sf);

            auto* pRoot = new gcroot<EventLoggerWrapper^>(this);
            _pGcRoot = pRoot;

            _native->SetCallback(EventLoggerNativeCallback{ pRoot });

            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "EventLoggerWrapper: Instance created");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "EventLoggerWrapper: Construction failed – " + std::string(ex.what()));
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

        NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
            "EventLoggerWrapper: Disposing");
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
    // Subscribe
    // -------------------------------------------------------------------------

    long EventLoggerWrapper::Subscribe()
    {
        CheckDisposed();
        try
        {
            long result = _native->Subscribe();
            if (result == 0)
            {
                _subscribed = true;
                NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                    "EventLoggerWrapper: Subscribed");
            }
            else
            {
                NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                    "EventLoggerWrapper: Subscribe() returned ADS error " +
                    std::to_string(result));
            }
            return result;
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "EventLoggerWrapper: Subscribe() threw – " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    // -------------------------------------------------------------------------
    // Unsubscribe
    // -------------------------------------------------------------------------

    void EventLoggerWrapper::Unsubscribe()
    {
        if (_disposed || _native == nullptr)
            return;

        try
        {
            _subscribed = false;
            _native->Unsubscribe();
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "EventLoggerWrapper: Unsubscribed");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "EventLoggerWrapper: Unsubscribe() threw – " + std::string(ex.what()));
        }
    }

    // -------------------------------------------------------------------------
    // ReadBacklog
    // -------------------------------------------------------------------------

    void EventLoggerWrapper::ReadBacklog()
    {
        CheckDisposed();
        if (!_subscribed)
        {
            throw gcnew InvalidOperationException(
                "ReadBacklog() cannot be called before Subscribe(). Call Subscribe() first to subscribe to EventLogger notifications.");
        }
        try
        {
            _native->ReadBacklog();
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
                "EventLoggerWrapper: ReadBacklog completed");
        }
        catch (const std::exception& ex)
        {
            NativeLogger::Instance().Log(NativeLogger::LogLevel::Error,
                "EventLoggerWrapper: ReadBacklog() threw – " + std::string(ex.what()));
            throw gcnew Exception(gcnew String(ex.what()));
        }
    }

    // -------------------------------------------------------------------------
    // OnNativeEvent – called from the ADS router thread via the native callback
    // -------------------------------------------------------------------------

    void EventLoggerWrapper::OnNativeEvent(const bhf::ads::TcEventEntry& entry,
                                           const std::string&             text,
                                           const std::string&             sourceName)
    {
        TcEventArgs^ args = gcnew TcEventArgs();
        args->Severity  = static_cast<TcEventSeverity>(entry.nSeverity);
        args->EventType = static_cast<EventTypeEnum>(entry.nEventType);
        args->ConfirmationState = static_cast<TcEventConfirmState>(entry.nConfirmationState);
        args->SourceId  = entry.nSrcId;
        args->EventId   = entry.nEventId;
        args->UniqueId  = entry.nUniqueId;
        args->SourceName = gcnew String(sourceName.c_str());

        array<System::Byte>^ guidBytes = gcnew array<System::Byte>(16);
        for (int i = 0; i < 16; ++i)
        {
            guidBytes[i] = entry.abEventClass[i];
        }
        args->EventClassGuid = System::Guid(guidBytes);

        args->TimeRaised = ConvertFileTimeOrFallback(entry.nTimeRaised, System::DateTime::UtcNow);
        args->TimeCleared = ConvertFileTimeOrFallback(entry.nTimeCleared, System::DateTime::MinValue);
        args->TimeConfirmed = ConvertFileTimeOrFallback(entry.nTimeConfirmed, System::DateTime::MinValue);

        args->SourceNameByteLength = entry.nCbSourceName;
        args->DataByteLength = entry.nCbData;
        
        args->Text      = gcnew String(text.c_str());

        EventReceived(this, args);
    }
}
