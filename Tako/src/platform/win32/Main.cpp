#include "Tako.hpp"
#include "World.hpp"
#include "Windows.h"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
#ifndef NDEBUG
	{
		AllocConsole();
		HWND hWnd = GetConsoleWindow();
		ShowWindow(hWnd, SW_SHOWNORMAL);
	}
#endif
	//tako::Archetype::Create<tako::Window, tako::Bitmap>();
	tako::World world;
	LOG("{}", world.Create());
	LOG("{}", world.Create<tako::Window>());
	tako::Window window;
	tako::GraphicsContext context(window.GetHandle(), window.GetWidth(), window.GetHeight());
	tako::Setup();
	tako::Broadcaster broadcaster;

	bool keepRunning = true;

	tako::CallbackEventHandler onEvent([&](tako::Event& ev)
		{
			switch (ev.GetType())
			{
			case tako::EventType::WindowClose:
			{
				tako::WindowClose& clo = static_cast<tako::WindowClose&>(ev);
				clo.abortQuit = false;
			} break;
			case tako::EventType::AppQuit:
			{
				keepRunning = false;
				LOG("Quitting...");
			} break;
			}

			LOG("Event: {}", ev);
		});

	broadcaster.Register(&onEvent);
	broadcaster.Register(&context);

	window.SetEventCallback([&](tako::Event& evt)
		{
			broadcaster.Broadcast(evt);
		});


	while (keepRunning)
	{
		window.Poll();
		context.Present();
		Sleep(16);
	}

	LOG("terminating")
		return 0;
}
