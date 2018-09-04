#pragma once

#define local_persist static

#include "Log.hpp"

#ifndef NDEBUG

#define LOG(format, ...)        ::tako::Log(tako::LogLevel::Info, format, ##__VA_ARGS__);
#define LOG_WARN(format, ...)   ::tako::Log(tako::LogLevel::Warning, format, ##__VA_ARGS__);
#define LOG_ERR(format, ...)    ::tako::Log(tako::LogLevel::Error, format, ##__VA_ARGS__);

#else

#define LOG(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERR(format, ...)    ::tako::Log(tako::LogLevel::Error, format, ##_VA_ARGS__);

#endif