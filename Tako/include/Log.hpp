#pragma once
#include <string_view>
#include "fmt/ostream.h"

namespace tako
{
    enum LogLevel
    {
        Info,
        Error,
        Warning
    };

    void PlatformConsoleLog(LogLevel level, const char* message);

    template<typename... Args>
    void Log(LogLevel level, const char* format, const Args&... args)
    {
        PlatformConsoleLog(level, fmt::format(format, args...).c_str());
    }
}