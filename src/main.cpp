#include <iostream>
#include "Windows.h"
#include "Window.hpp"
#include "Utility.hpp"
#include "FileSystem.hpp"
#include "Bitmap.hpp"
#include <vector>
#include <gl/GL.h>

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
    tako::Bitmap map(160, 144);
    map.Clear("#F00"_col);
    map.FillRect(64, 64, 16, 16, "#00F"_col);

    while (!window.ShouldExit())
    {
        window.Poll();
        window.DrawBitmap(map);
        Sleep(16);
    }
    
    return 0;
}