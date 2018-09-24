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
    int w = 32, h = 32;
    tako::Bitmap map(w, h);
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            float hue = (float) x / w;
            float saturation = 1 - ((float)y / h);
            map.SetPixel(x, y, tako::Color::FromHSL(hue, saturation, 0.5f));
        }
    }

    while (!window.ShouldExit())
    {
        window.Poll();
        window.DrawBitmap(map);
        Sleep(16);
    }
    
    return 0;
}