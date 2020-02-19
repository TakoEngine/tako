#include "Tako.hpp"
#include "World.hpp"
#include "Windows.h"
#include <chrono>

struct Position
{
	tako::Vector2 pos;
};

struct Velocity
{
	tako::Vector2 vel;
};

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
