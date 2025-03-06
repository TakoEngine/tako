#pragma once
#include <string_view>
#include "fmt/format.h"

namespace tako
{
	enum LogLevel
	{
		Info,
		Error,
		Warning
	};

	void PlatformConsoleLog(LogLevel level, const char* message);

	inline void VLog(LogLevel level, fmt::string_view format, fmt::format_args args)
	{
		PlatformConsoleLog(level, fmt::vformat(format, args).c_str());
	}

	template<typename... Args>
	void Log(LogLevel level, fmt::format_string<Args...> format, const Args& ... args)
	{
		VLog(level, format, fmt::make_format_args(args...));
	}

	/*
	template<typename... Args>
	void Log(LogLevel level, std::string_view format, const Args& ... args)
	{
		PlatformConsoleLog(level, fmt::format(fmt::runtime(format), args...).c_str());
	}
	*/
}
