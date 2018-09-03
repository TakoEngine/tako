#include "Log.hpp"
#include <iostream>
#include <Windows.h>

namespace tako
{
    void PlatformConsoleLog(tako::LogLevel level, std::string_view message)
    {
        static HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
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

        std::cout << message << std::endl;
    }
}
