#include "Utility.hpp"
#include "Log.hpp"
#include <iostream>

namespace tako
{
    void PlatformConsoleLog(tako::LogLevel level, const char* message)
    {
        std::cout << message << std::endl;
    }
}


