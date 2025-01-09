#pragma once

#define local_persist static

#include <cassert>

#define ASSERT(condition) assert(condition)

#include "Log.hpp"

#if !defined(NDEBUG) || defined(TAKO_FORCE_LOG)

#define LOG(format, ...)        ::tako::Log(tako::LogLevel::Info, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)   ::tako::Log(tako::LogLevel::Warning, format, ##__VA_ARGS__)
#define LOG_ERR(format, ...)    ::tako::Log(tako::LogLevel::Error, format, ##__VA_ARGS__)

#else

#define LOG(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERR(format, ...)    ::tako::Log(tako::LogLevel::Error, format, ##__VA_ARGS__)

#endif
