#pragma once

#include "NativeLogger.h"
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace AdsWrapper
{
    public enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error
    };

    public delegate void LogMessageHandler(LogLevel level, String^ message);

    public ref class LoggerWrapper
    {
    public:
        static void SetCallback(LogMessageHandler^ handler);

    internal:  // was private — native lambdas need at least internal access
        static void NativeCallback(NativeLogger::LogLevel level,
            const std::string& message);

    private:
        static LogMessageHandler^ _managedHandler;
        static GCHandle _gcHandle;
    };
}