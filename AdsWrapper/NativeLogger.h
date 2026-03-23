#pragma once

#include <functional>
#include <string>
#include <mutex>

/// <summary>
/// Singleton logger class that provides thread-safe logging functionality.
/// Supports logging at different levels and allows custom callback handlers.
/// </summary>
class NativeLogger
{
public:
    /// <summary>
    /// Defines the severity levels for log messages.
    /// </summary>
    enum class LogLevel
    {
        Debug,      ///< Detailed information for debugging purposes
        Info,       ///< General informational messages
        Warning,    ///< Warning messages for potentially problematic situations
        Error       ///< Error messages for failures and exceptions
    };

    /// <summary>
    /// Callback function signature for handling log messages.
    /// </summary>
    /// <param name="level">The severity level of the log message</param>
    /// <param name="message">The log message content</param>
    using LogCallback =
        std::function<void(LogLevel, const std::string&)>;

    /// <summary>
    /// Gets the singleton instance of the logger.
    /// </summary>
    /// <returns>Reference to the singleton NativeLogger instance</returns>
    static NativeLogger& Instance();

    /// <summary>
    /// Sets the callback function that will receive log messages.
    /// </summary>
    /// <param name="callback">The callback function to handle log messages</param>
    void SetCallback(LogCallback callback);

    /// <summary>
    /// Logs a message at the specified level.
    /// Thread-safe operation protected by mutex.
    /// </summary>
    /// <param name="level">The severity level of the message</param>
    /// <param name="message">The message to log</param>
    void Log(LogLevel level, const std::string& message);

private:
    NativeLogger() = default;

    LogCallback _callback;  ///< The registered callback function
    std::mutex _mutex;      ///< Mutex for thread-safe logging
};
