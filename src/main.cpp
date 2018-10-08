#include <iostream>
#include "Windows.h"
#include "Window.hpp"
#include "GraphicsContext.hpp"
#include "Utility.hpp"
#include "FileSystem.hpp"

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
    tako::GraphicsContext context(window);

    while (!window.ShouldExit())
    {
        window.Poll();
        context.Present();
        Sleep(16);
    }
    
    LOG("terminating")
    return 0;
}