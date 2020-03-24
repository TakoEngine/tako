#include <Windows.h>
#include "EntryPoint.hpp"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
#ifndef NDEBUG
	{
		AllocConsole();
		HWND hWnd = GetConsoleWindow();
		ShowWindow(hWnd, SW_SHOWNORMAL);
	}
#endif
    return tako::RunGameLoop();
}
