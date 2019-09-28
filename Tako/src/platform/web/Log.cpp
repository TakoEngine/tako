#include "Utility.hpp"
#include "Log.hpp"
#include <emscripten.h>

namespace tako
{
	namespace
	{
		int ToEMLog(tako::LogLevel level)
		{
			switch (level)
			{
			case tako::LogLevel::Error: return EM_LOG_ERROR;
			case tako::LogLevel::Info: return EM_LOG_CONSOLE;
			case tako::LogLevel::Warning: return EM_LOG_WARN;
			}
		}
	}
	void PlatformConsoleLog(tako::LogLevel level, const char* message)
	{
		emscripten_log(ToEMLog(level), message);
	}
}
