import EntryPoint;

#include <Windows.h>

int main(int argc, char* argv[])
{
#ifndef NDEBUG
	{
		AllocConsole();
		HWND hWnd = GetConsoleWindow();
		ShowWindow(hWnd, SW_SHOWNORMAL);
	}
#endif
	return tako::RunGameLoop(argc, argv);
}
