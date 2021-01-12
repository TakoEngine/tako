#include "EntryPoint.hpp"
#include "Tako.hpp"
#include "World.hpp"
#include "Timer.hpp"
#include "Resources.hpp"
#include "OpenGLPixelArtDrawer.hpp"
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
#ifdef TAKO_EDITOR
#include "FileWatcher.hpp"
#endif

namespace tako
{
	struct TickStruct
	{
		tako::Window& window;
		tako::GraphicsContext& context;
		tako::PixelArtDrawer* drawer;
		tako::Input& input;
		tako::Resources& resources;
#ifdef TAKO_EDITOR
		tako::FileWatcher& watcher;
#endif
	};

	void Tick(void* p)
	{
		TickStruct* data = reinterpret_cast<TickStruct*>(p);
		static tako::Timer timer;
		float dt = timer.GetDeltaTime();
#ifdef TAKO_EDITOR
		for (auto& change: data->watcher.Poll())
		{
			LOG("Change {}", change.path);
			LOG("Change: {}", change.path.filename());
			if (change.path.extension() == ".png")
			{
				auto file = "/" / change.path.filename();
				auto bitmap = Bitmap::FromFile(file.c_str());
				data->drawer->UpdateTexture(data->resources.Load<Texture>(file), bitmap);
			}
		}
#endif
		data->window.Poll();
		data->input.Update();
		tako::Update(&data->input, dt);
		tako::Draw(data->drawer);
		data->context.Present();
	}

	int RunGameLoop()
	{
		LOG("Init!");
		auto api = tako::GraphicsAPI::OpenGL;
		tako::Window window(api);
		tako::Input input;
		auto context = CreateGraphicsContext(&window, api);
		auto drawer = new OpenGLPixelArtDrawer(context.get());
		drawer->Resize(window.GetWidth(), window.GetHeight());
		Audio audio;
		audio.Init();
		Resources resources(context.get());
		tako::Setup(drawer, &resources);
		tako::Broadcaster broadcaster;
#ifdef TAKO_EDITOR
		tako::FileWatcher watcher("./Assets");
#endif

		bool keepRunning = true;
		tako::CallbackEventHandler onEvent([&](tako::Event& ev)
		{
			LOG("Event: {}", ev);
			switch (ev.GetType())
			{
				case tako::EventType::WindowResize:
				{
					tako::WindowResize& res = static_cast<tako::WindowResize&>(ev);
					drawer->Resize(res.width, res.height);
				} break;
				case tako::EventType::WindowClose:
				{
					tako::WindowClose& clo = static_cast<tako::WindowClose&>(ev);
					clo.abortQuit = false;

					// For now, send app quit event, later handle callbacks to maybe abort closing process
					tako::AppQuit quitEvent;
					broadcaster.Broadcast(quitEvent);
				} break;
				case tako::EventType::AppQuit:
				{
					keepRunning = false;
					LOG("Quitting...");
				} break;
			}
		});

		broadcaster.Register(&onEvent);
		broadcaster.Register(context.get());
		broadcaster.Register(&input);

		window.SetEventCallback([&](tako::Event& evt)
		{
			broadcaster.Broadcast(evt);
		});

		TickStruct data
		{
			window,
			*context,
			drawer,
			input,
			resources,
#ifdef TAKO_EDITOR
			watcher
#endif
		};

#ifndef EMSCRIPTEN
		while (!window.ShouldExit() && keepRunning)
		{
			Tick(&data);
		}
#else
		emscripten_set_main_loop_arg(Tick, &data, 0, 1);
#endif


		LOG("terminating");
		return 0;
	}
}
