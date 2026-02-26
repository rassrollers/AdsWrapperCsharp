#include "pch.h"
#include "LoggerWrapper.h"

#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

namespace AdsWrapper
{
    void LoggerWrapper::SetCallback(LogMessageHandler^ handler)
    {
        // Release previous handle if any
        if (_gcHandle.IsAllocated)
            _gcHandle.Free();

        _managedHandler = handler;

        if (handler != nullptr)
        {
            // Pin the delegate so the GC does not collect it
            _gcHandle = GCHandle::Alloc(_managedHandler);

            // Register a native callback that forwards to the managed delegate
            NativeLogger::Instance().SetCallback(
                [](NativeLogger::LogLevel level, const std::string& msg)
                {
                    NativeCallback(level, msg);
                });
        }
        else
        {
            NativeLogger::Instance().SetCallback(nullptr);
        }
    }

    void LoggerWrapper::NativeCallback(NativeLogger::LogLevel level,
        const std::string& message)
    {
        if (_managedHandler != nullptr)
        {
            // Convert native LogLevel to managed LogLevel
            LogLevel managedLevel;
            switch (level)
            {
            case NativeLogger::LogLevel::Debug:
                managedLevel = LogLevel::Debug;
                break;
            case NativeLogger::LogLevel::Info:
                managedLevel = LogLevel::Info;
                break;
            case NativeLogger::LogLevel::Warning:
                managedLevel = LogLevel::Warning;
                break;
            case NativeLogger::LogLevel::Error:
                managedLevel = LogLevel::Error;
                break;
            default:
                managedLevel = LogLevel::Info;
                break;
            }

            // Convert std::string to System::String^
            String^ managedMessage = gcnew String(message.c_str());

            _managedHandler->Invoke(managedLevel, managedMessage);
        }
    }
}