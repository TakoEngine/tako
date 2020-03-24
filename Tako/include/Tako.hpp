#pragma once

#include <iostream>
//
#include "Window.hpp"
#include "GraphicsContext.hpp"
#include "Utility.hpp"
#include "FileSystem.hpp"
#include "Input.hpp"
//#include <emscripten.h>
#include "Audio.hpp"

namespace tako
{
	extern void Setup(PixelArtDrawer* drawer);
	extern void Update(Input* input, float dt);
	extern void Draw(PixelArtDrawer* drawer);

#ifdef TAKO_OPENGL
	static tako::PixelArtDrawer* Graphics;
#endif
}
/*
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
*/

/*
int main()
{
	tako::Window window;
	tako::GraphicsContext context(window.GetHandle(), window.GetWidth(), window.GetHeight());
	tako::Input input;
    tako::Broadcaster broadcaster;
#ifdef TAKO_OPENAL
	tako::Audio audio;
	audio.Init();
#endif
	

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
*/
