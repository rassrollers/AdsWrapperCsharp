#include "pch.h"
#include "NativeLogger.h"

NativeLogger& NativeLogger::Instance()
{
    static NativeLogger instance;
    return instance;
}

void NativeLogger::SetCallback(LogCallback callback)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _callback = callback;
}

void NativeLogger::Log(LogLevel level,
    const std::string& message)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_callback)
        _callback(level, message);
}
