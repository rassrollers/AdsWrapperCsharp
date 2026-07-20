#pragma once

using namespace System;

namespace AdsWrapper
{
    /// <summary>Severity levels for TwinCAT EventLogger events.</summary>
    public enum class TcEventSeverity : System::UInt16
    {
        Verbose = 0,
        Info = 1,
        Warning = 2,
        Error = 3,
        Critical = 4,
    };

    /// <summary>Discriminates one-shot message events from alarm events.</summary>
    public enum class EventTypeEnum : System::UInt16
    {
        Alarm = 0,
        Message = 1,
    };

    /// <summary>Confirmation state of an alarm event.</summary>
    public enum class TcEventConfirmState : System::UInt16
    {
        Confirmed = 0,
        NotRequired = 1,
        NotSupported = 2,
        Reset = 3,
        WaitForConfirm = 4,
    };

    /// <summary>
    /// Event data delivered by <see cref="EventLoggerWrapper::EventReceived"/>.
    /// </summary>
    public ref class TcEventArgs : System::EventArgs
    {
    public:
        /// <summary>Severity level of the event.</summary>
        property TcEventSeverity Severity;

        /// <summary>Distinguishes message events from alarm events.</summary>
        property EventTypeEnum EventType;

        /// <summary>Identifies the source device or task that raised the event.</summary>
        property System::UInt32 SourceId;

        /// <summary>Application-defined event identifier.</summary>
        property System::UInt32 EventId;

        /// <summary>Event class GUID from the EventLogger record.</summary>
        property System::Guid EventClassGuid;

        /// <summary>Event payload UniqueId field.</summary>
        property System::UInt32 UniqueId;

        /// <summary>Event payload SourceName string.</summary>
        property System::String^ SourceName;

        /// <summary>UTC timestamp when the event was raised.</summary>
        property System::DateTime TimeRaised;

        /// <summary>UTC timestamp when the event was cleared (MinValue when unavailable).</summary>
        property System::DateTime TimeCleared;

        /// <summary>UTC timestamp when the event was confirmed (MinValue when unavailable).</summary>
        property System::DateTime TimeConfirmed;

        /// <summary>Confirmation state for alarm events.</summary>
        property TcEventConfirmState ConfirmationState;

        /// <summary>Source name length in bytes from the native record.</summary>
        property System::UInt32 SourceNameByteLength;

        /// <summary>Payload length in bytes from the native record.</summary>
        property System::UInt32 DataByteLength;

        /// <summary>
        /// Decoded event text.  Empty when the publisher did not attach a text payload.
        /// </summary>
        property System::String^ Text;
    };
}