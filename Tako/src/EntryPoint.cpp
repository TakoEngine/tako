#include "EntryPoint.hpp"
#include "Tako.hpp"
#include "World.hpp"
#include "Timer.hpp"
#include "Resources.hpp"
//#include "OpenGLPixelArtDrawer.hpp"
#include "Renderer3D.hpp"
#include "JobSystem.hpp"
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
		tako::Input& input;
		tako::Resources& resources;
		void* gameData;
		GameConfig& config;
		JobSystem& jobSys;
		std::atomic<bool>& keepRunning;
#ifdef TAKO_EDITOR
		tako::FileWatcher& watcher;
#endif
	};

	void Tick(void* p)
	{
		LOG("Tick Start");
		static tako::Timer timer;
		float dt = timer.GetDeltaTime();
		LOG("dt {}", dt);
		TickStruct* data = reinterpret_cast<TickStruct*>(p);
/*
#ifdef TAKO_EDITOR
		for (auto& change: data->watcher.Poll())
		{
			LOG("Change {}", change.path);
			LOG("Change: {}", change.path.filename());
#ifdef TAKO_OPENGL
			if (change.path.extension() == ".png")
			{
				auto file = "/" / change.path.filename();
				auto bitmap = Bitmap::FromFile(file.c_str());
				data->drawer->UpdateTexture(data->resources.Load<Texture>(file), bitmap);
			}
#endif
		}
#endif
 */
		data->window.Poll();
		data->input.Update();

		data->jobSys.Schedule([=]()
		{
			void* frameData = malloc(data->config.frameDataSize);
			GameStageData stageData
			{
				data->gameData,
				frameData
			};
			if (data->config.Update)
			{
				data->config.Update(stageData, &data->input, dt);
			}
			if (data->window.ShouldExit() || !data->keepRunning)
			{
				data->jobSys.Stop();
				return;
			}
			data->jobSys.ScheduleForThread(0, std::bind(Tick, p));
			//data->context.Begin();
			//data->gameData->
			if (data->config.Draw)
			{
				data->config.Draw(stageData);
			}
			free(frameData);
			//data->context.End();
			data->jobSys.ScheduleForThread(0, [=]()
			{
				data->context.Present();
				LOG("Tick End");
			});
		});
	}

	int RunGameLoop()
	{
		LOG("Init!");
		JobSystem jobSys;
		jobSys.Init();
		Audio audio;
		jobSys.ScheduleForThread(0,[&]()
		{
			audio.Init();
			LOG("Audio initialized!");
		});
		GameConfig config = {};
		tako::InitTakoConfig(config);
		auto api = tako::ResolveGraphicsAPI(config.graphicsAPI);
		tako::Window window(api);
		tako::Input input;
		auto context = CreateGraphicsContext(&window, api);
		//TODO: Get frontend to get from configuration	
		//auto drawer = new OpenGLPixelArtDrawer(context.get());
		//drawer->Resize(window.GetWidth(), window.GetHeight());
		
		Resources resources(context.get());
		void* gameData = malloc(config.gameDataSize);
		if (config.Setup)
		{
			config.Setup(gameData, { context.get(), &resources, &audio });
		}
		tako::Broadcaster broadcaster;
#ifdef TAKO_EDITOR
		tako::FileWatcher watcher("./Assets");
#endif

		std::atomic<bool> keepRunning = true;
		tako::CallbackEventHandler onEvent([&](tako::Event& ev)
		{
			//LOG("Event: {}", ev);
			switch (ev.GetType())
			{
				case tako::EventType::WindowResize:
				{
					tako::WindowResize& res = static_cast<tako::WindowResize&>(ev);
					//drawer->Resize(res.width, res.height);
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
			input,
			resources,
			gameData,
			config,
			jobSys,
			keepRunning
#ifdef TAKO_EDITOR
			watcher
#endif
		};

#ifndef EMSCRIPTEN
		/*
		while (!window.ShouldExit() && keepRunning)
		{
			jobSys.Schedule([]() {LOG("it works")});
			Tick(&data);
		}
		*/
		jobSys.Schedule(std::bind(Tick, &data));
		jobSys.JoinAsWorker();
#else
		emscripten_set_main_loop_arg(Tick, &data, 0, 1);
#endif


		LOG("terminating");
		return 0;
	}
}
