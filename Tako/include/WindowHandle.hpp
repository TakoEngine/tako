#pragma once
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif
#ifdef TAKO_GLFW
#include <GLFW/glfw3.h>
#endif



namespace tako
{
#ifdef __EMSCRIPTEN__
	using WindowHandle = const char*;
#endif
#if defined(TAKO_GLFW)
	using WindowHandle = GLFWwindow*;
#elif defined(_WIN32)
	using WindowHandle = HWND;
#endif



}
