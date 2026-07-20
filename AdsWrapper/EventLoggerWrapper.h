#pragma once

#include "AdsDefines.h"
#include "EventLoggerAccess.h"

using namespace System;
using namespace System::Runtime::InteropServices;

class NativeEventLogger;  // forward declaration

namespace AdsWrapper
{
    /// <summary>
    /// Managed .NET wrapper for subscribing to TwinCAT EventLogger Publisher V2 notifications.
    /// Raises <see cref="EventReceived"/> on the ADS router thread for every event received
    /// from the remote TwinCAT system.  Call <see cref="Start"/> to subscribe and
    /// <see cref="Stop"/> (or Dispose) to unsubscribe.
    /// </summary>
    public ref class EventLoggerWrapper : IDisposable
    {
    public:
        /// <summary>
        /// Initializes a new EventLogger subscription on the specified AMS port.
        /// </summary>
        /// <param name="gateway">IP address or hostname of the AMS router gateway</param>
        /// <param name="netId">AMS NetId of the target TwinCAT system (e.g. "192.168.0.1.1.1")</param>
        /// <param name="port">AMS port of the EventLogger Publisher (use <see cref="AmsPort::EventLoggerPublisher"/> = 132)</param>
        /// <param name="severityFilter">
        /// Minimum severity passed as IndexOffset to the subscription:
        /// 0 = all (Verbose+), 1 = Info+, 2 = Warning+, 4 = Error+, 8 = Critical only.
        /// </param>
        EventLoggerWrapper(String^ gateway, String^ netId, AmsPort port,
                           System::UInt32 severityFilter);

        /// <summary>
        /// Initializes a new EventLogger subscription on the specified AMS port, receiving all severities.
        /// </summary>
        /// <param name="gateway">IP address or hostname of the AMS router gateway</param>
        /// <param name="netId">AMS NetId of the target TwinCAT system (e.g. "192.168.0.1.1.1")</param>
        /// <param name="port">AMS port of the EventLogger Publisher (use <see cref="AmsPort::EventLoggerPublisher"/> = 132)</param>
        EventLoggerWrapper(String^ gateway, String^ netId, AmsPort port);

        /// <summary>
        /// Initializes a new EventLogger subscription on the default EventLogger Publisher port (132).
        /// </summary>
        /// <param name="gateway">IP address or hostname of the AMS router gateway</param>
        /// <param name="netId">AMS NetId of the target TwinCAT system (e.g. "192.168.0.1.1.1")</param>
        EventLoggerWrapper(String^ gateway, String^ netId);

        /// <summary>Managed destructor. Disposes managed and unmanaged resources.</summary>
        ~EventLoggerWrapper();

        /// <summary>Finalizer. Releases unmanaged resources if Dispose was not called.</summary>
        !EventLoggerWrapper();

        /// <summary>
        /// Raised on the ADS router thread for every TwinCAT event received after
        /// <see cref="Start"/> is called.
        /// </summary>
        event EventHandler<TcEventArgs^>^ EventReceived;

        /// <summary>
        /// Subscribes to EventLogger notifications.  Non-blocking; events are delivered
        /// asynchronously via <see cref="EventReceived"/>.
        /// </summary>
        /// <returns>0 on success, or a non-zero ADS error code on failure.</returns>
        /// <exception cref="ObjectDisposedException">Thrown if the object has been disposed.</exception>
        long Start();

        /// <summary>
        /// Unsubscribes from EventLogger notifications.
        /// Safe to call even if <see cref="Start"/> was not called or already stopped.
        /// </summary>
        void Stop();

    private:
        NativeEventLogger* _native;
        bool               _disposed;
        void*              _pGcRoot;   ///< Heap-allocated gcroot<EventLoggerWrapper^>

        void CheckDisposed();

    internal:
        void OnNativeEvent(const bhf::ads::TcEventEntry& entry,
                           const std::string&             text,
                           uint32_t                       uniqueId,
                           const std::string&             sourceName);
    };
}
