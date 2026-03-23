#pragma once

#include "NativeLogger.h"
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace AdsWrapper
{
    /// <summary>
    /// Defines the severity levels for log messages in managed code.
    /// </summary>
    public enum class LogLevel
    {
        Debug,      ///< Detailed information for debugging purposes
        Info,       ///< General informational messages
        Warning,    ///< Warning messages for potentially problematic situations
        Error       ///< Error messages for failures and exceptions
    };

    /// <summary>
    /// Delegate for handling log messages from the native ADS library.
    /// </summary>
    /// <param name="level">The severity level of the log message</param>
    /// <param name="message">The log message content</param>
    public delegate void LogMessageHandler(LogLevel level, String^ message);

    /// <summary>
    /// Managed wrapper class that bridges native C++ logging to .NET delegates.
    /// Allows .NET applications to receive log messages from the native ADS library.
    /// </summary>
    public ref class LoggerWrapper
    {
    public:
        /// <summary>
        /// Sets the managed callback handler for receiving log messages.
        /// </summary>
        /// <param name="handler">The delegate that will receive log messages</param>
        static void SetCallback(LogMessageHandler^ handler);

    internal:  // Native lambdas need at least internal access
        /// <summary>
        /// Internal callback that receives messages from native code and forwards to managed handler.
        /// </summary>
        /// <param name="level">Native log level</param>
        /// <param name="message">Native log message</param>
        static void NativeCallback(NativeLogger::LogLevel level,
            const std::string& message);

    private:
        static LogMessageHandler^ _managedHandler;  ///< The registered managed callback
        static GCHandle _gcHandle;                  ///< GC handle to prevent garbage collection
    };
}