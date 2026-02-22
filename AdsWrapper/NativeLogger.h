#pragma once

#include <functional>
#include <string>
#include <mutex>

class NativeLogger
{
public:
    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error
    };

    using LogCallback =
        std::function<void(LogLevel, const std::string&)>;

    static NativeLogger& Instance();

    void SetCallback(LogCallback callback);
    void Log(LogLevel level, const std::string& message);

private:
    NativeLogger() = default;

    LogCallback _callback;
    std::mutex _mutex;
};
