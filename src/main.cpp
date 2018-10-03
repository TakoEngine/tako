#include <iostream>
#include "Windows.h"
#include "Window.hpp"
#include "Utility.hpp"
#include "FileSystem.hpp"

#include <vulkan/vulkan.h>

using namespace tako::literals;

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
#ifndef NDEBUG
    {
        AllocConsole();
        HWND hWnd = GetConsoleWindow();
        ShowWindow(hWnd, SW_SHOWNORMAL);
    }
#endif
    tako::Window window;

    while (!window.ShouldExit())
    {
        window.Poll();
        Sleep(16);
    }
    
    LOG("terminating")
    return 0;
}