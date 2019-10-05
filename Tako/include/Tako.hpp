#pragma once

#include <iostream>
//#include "Windows.h"
#include "Window.hpp"
#include "GraphicsContext.hpp"
#include "Utility.hpp"
#include "FileSystem.hpp"
#include "Input.hpp"
#include <emscripten.h>

namespace tako
{
	extern void Setup(PixelArtDrawer* drawer);
	extern void Update(Input* input, float dt);
	extern void Draw(PixelArtDrawer* drawer);

    static tako::PixelArtDrawer* Graphics;
}

struct UpdateStruct
{
    tako::PixelArtDrawer* drawer;
    tako::Input* input;
};

struct TickStruct
{
	tako::Window* window;
	tako::GraphicsContext* context;
	tako::PixelArtDrawer* drawer;
	tako::Input* input;
	double lastFrame = 0;
};

void Tick(void* p)
{
	TickStruct* data = reinterpret_cast<TickStruct*>(p);
    double time = emscripten_get_now();
    float dt = (time - data->lastFrame) / 1000;
	data->window->Poll();
	data->input->Update();
    tako::Update(data->input, dt);
	tako::Draw(data->drawer);
	data->context->Present();
	data->lastFrame = time;
}

int main()
{
	tako::Window window;
	tako::GraphicsContext context(window.GetHandle(), window.GetWidth(), window.GetHeight());
	tako::Input input;
    tako::Broadcaster broadcaster;

    broadcaster.Register(&context);
    broadcaster.Register(&input);

    window.SetEventCallback([&](tako::Event& evt)
    {
        broadcaster.Broadcast(evt);
    });

	TickStruct data;
	data.window = &window;
	data.context = &context;
	data.drawer = context.CreatePixelArtDrawer();
	data.input = &input;
	tako::Graphics = data.drawer;
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
