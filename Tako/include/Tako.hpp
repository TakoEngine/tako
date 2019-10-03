#pragma once

#include <iostream>
//#include "Windows.h"
#include "Window.hpp"
#include "GraphicsContext.hpp"
#include "Utility.hpp"
#include "FileSystem.hpp"
#include <emscripten.h>

namespace tako
{
	extern void Setup(PixelArtDrawer* drawer);
	extern void Update();
	extern void Draw(PixelArtDrawer* drawer);
}

struct TickStruct
{
	tako::Window* window;
	tako::GraphicsContext* context;
	tako::PixelArtDrawer* drawer;
};

void Tick(void* p)
{
	TickStruct* data = reinterpret_cast<TickStruct*>(p);
	data->window->Poll();
    tako::Update();
	tako::Draw(data->drawer);
	data->context->Present();
}

int main()
{
	tako::Window window;
	tako::GraphicsContext context(window.GetHandle(), 1024, 768);
    tako::Broadcaster broadcaster;

    broadcaster.Register(&context);

    window.SetEventCallback([&](tako::Event& evt)
    {
        broadcaster.Broadcast(evt);
    });

	TickStruct data;
	data.window = &window;
	data.context = &context;
	data.drawer = context.CreatePixelArtDrawer();
    tako::Setup(data.drawer);
	emscripten_set_main_loop_arg(Tick, &data, 0, 1);
	return 0;
}
/*
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
*/
