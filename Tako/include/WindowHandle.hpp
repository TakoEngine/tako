#pragma once
#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef __EMSCRIPTEN__
#include <GLFW/glfw3.h>
#endif
#ifdef TAKO_GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif



namespace tako
{
#ifdef __EMSCRIPTEN__
	using WindowHandle = GLFWwindow*;
#endif
#ifdef _WIN32
	using WindowHandle = HWND;
#endif
#ifdef TAKO_GLFW
	using WindowHandle = GLFWwindow*;
#endif

	
}
