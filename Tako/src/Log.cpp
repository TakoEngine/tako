#include "Utility.hpp"
#include "Log.hpp"
#include <iostream>
#include <Windows.h>

namespace tako
{
    void PlatformConsoleLog(tako::LogLevel level, const char* message)
    {
        local_persist HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        const char* newLine = "\n";
        switch (level)
        {
        case LogLevel::Info:
            SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
            break;
        case LogLevel::Warning:
            SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN);
            break;
        case LogLevel::Error:
            SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_INTENSITY);
            break;
        }

        DWORD written;
        WriteConsole(console, message, strlen(message), &written, NULL);
        WriteConsole(console, newLine, 1, &written, NULL);
#ifndef NDEBUG
        OutputDebugString(message);
        OutputDebugString("\n");
#endif
    }
}
