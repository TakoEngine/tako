module;
#include "Tako.hpp"
#include "Timer.hpp"
#include "Resources.hpp"
#include <thread>
//#include "OpenGLPixelArtDrawer.hpp"
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/proxying.h>
#endif
#ifdef TAKO_EDITOR
#include "FileWatcher.hpp"
#endif

#ifdef TAKO_IMGUI
#include "imgui.h"
#ifdef TAKO_WIN32
#include "imgui_impl_win32.h"
#endif
#ifdef TAKO_GLFW
#include "imgui_impl_glfw.h"
#endif
#include "imgui_impl_opengl3.h"
#endif

export module EntryPoint;

import Tako.Audio;
//import Tako.Renderer3D;
import Tako.Application;
import Tako.Serialization;
import Tako.JobSystem;
import Tako.Allocators.PoolAllocator;

namespace tako
{
	struct TickStruct
	{
		tako::Window& window;
		tako::GraphicsContext& context;
		tako::Input& input;
		tako::Audio& audio;
		tako::Resources& resources;
		void* gameData;
		GameConfig& config;
		JobSystem& jobSys;
		std::atomic<bool>& keepRunning;
		
		Allocators::PoolAllocator frameDataPool;
#ifdef EMSCRIPTEN
		pthread_t mainThread;
		emscripten::ProxyingQueue proxyQueue;
#endif
		std::atomic_flag frameDataPoolLock = ATOMIC_FLAG_INIT;
		std::atomic<int> frame = 0;
		int openFrames = 1;
		std::atomic_flag frameCounterLock = ATOMIC_FLAG_INIT;
#ifdef TAKO_EDITOR
		tako::FileWatcher& watcher;
#endif
	};

	void Tick(void* p)
	{
		//LOG("Tick Start");
		static tako::Timer timer;
		constexpr auto maxFramesConcurrent = 2;
		float dt = timer.GetDeltaTime();
		TickStruct* data = reinterpret_cast<TickStruct*>(p);
		auto thisFrame = ++data->frame;
		static std::atomic<float> fps = 1;
		fps = 0.01f * 1/dt + 0.99f * fps;
		//LOG("frame {}: fps: {}", thisFrame, 1/dt);
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
		data->jobSys.Schedule([=]()
		{
			data->window.Poll();
#ifdef TAKO_IMGUI
			ImGui_ImplOpenGL3_NewFrame();
#ifdef TAKO_WIN32
			ImGui_ImplWin32_NewFrame();
#endif
#ifdef TAKO_GLFW
			ImGui_ImplGlfw_NewFrame();
#endif
			ImGui::NewFrame();
#endif
			data->input.Update();
		});

		/*
		while (data->frameDataPoolLock.test_and_set(std::memory_order_acquire));
		void* frameData = data->frameDataPool.Allocate();
		data->frameDataPoolLock.clear(std::memory_order_release);
		*/
		void* frameData = malloc(data->config.frameDataSize);

		data->jobSys.Continuation([=]()
		{
			if (data->config.Update)
			{
				data->jobSys.Schedule([=]()
				{
					GameStageData stageData
					{
						data->gameData,
						frameData
					};
					data->config.Update(stageData, &data->input, dt);
				});
			}
			data->jobSys.Continuation([=]()
			{
				if (data->window.ShouldExit() || !data->keepRunning)
				{
					data->jobSys.Stop();
					return;
				}
				//LOG("Start Draw {}", thisFrame);
				//data->jobSys.Schedule([=]()
				data->proxyQueue.proxySync(data->mainThread, [=]()
				{
					data->context.Begin();
					if (data->config.Draw)
					{
						GameStageData stageData
						{
							data->gameData,
							frameData
						};
						data->config.Draw(stageData);
					}
					data->context.End();
#ifdef TAKO_IMGUI
					ImGui::EndFrame();
					ImGui::Render();
					ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

					if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
					{
						ImGui::UpdatePlatformWindows();
						ImGui::RenderPlatformWindowsDefault();
					}
#endif
					data->context.Present();
					//LOG("Tick End");
				});

				#ifndef EMSCRIPTEN
					data->jobSys.ScheduleDetached(std::bind(Tick, p));
				#endif

				data->jobSys.Continuation([=]()
				{
					/*
					while (data->frameDataPoolLock.test_and_set(std::memory_order_acquire));
					data->frameDataPool.Deallocate(frameData);
					data->frameDataPoolLock.clear(std::memory_order_release);
					*/
					free(frameData);
				});
			});
		});

	}

	void EmscriptenDelayed(void* p)
	{
		TickStruct* data = reinterpret_cast<TickStruct*>(p);
		data->jobSys.Init();
		if (data->config.Setup)
		{
			data->jobSys.RunJob([=]()
			{
				data->config.Setup(data->gameData, { &data->context, &data->resources, &data->audio });
			});
		}
	}

	void ScheduleTick(void* p)
	{
		TickStruct* data = reinterpret_cast<TickStruct*>(p);
		data->jobSys.RunJob(std::bind(Tick, p));
	}

	export int RunGameLoop(int argc, char* argv[])
	{
		LOG("Init!");
		Application::argc = argc;
		Application::argv = argv;
		JobSystem jobSys;
		#ifndef EMSCRIPTEN
			jobSys.Init();
		#endif
		GameConfig config = {};
		tako::InitTakoConfig(config);

		Audio audio;
		//TODO: jobify
		if (!config.initAudioDelayed)
		{
			audio.Init();
			LOG("Audio initialized!");
		};
		auto api = tako::ResolveGraphicsAPI(config.graphicsAPI);
		tako::Window window(api);
		tako::Input input;
		auto context = CreateGraphicsContext(&window, api);
		//TODO: Get frontend to get from configuration
		//auto drawer = new OpenGLPixelArtDrawer(context.get());
		//drawer->Resize(window.GetWidth(), window.GetHeight());

#ifdef TAKO_IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		auto& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#ifdef TAKO_WIN32
		ImGui_ImplWin32_Init(window.GetHandle());
#endif
#ifdef TAKO_GLFW
		ImGui_ImplGlfw_InitForOpenGL(window.GetHandle(), true);
#endif
		ImGui_ImplOpenGL3_Init();
#endif

		Resources resources(context.get());
		void* gameData = malloc(config.gameDataSize);
#ifndef EMSCRIPTEN
		if (config.Setup)
		{
			config.Setup(gameData, { context.get(), &resources, &audio });
		}
#endif
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
					LOG("Resize {} {}", res.width, res.height);
					context->Resize(res.width, res.height);
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

		size_t framePoolSize = config.frameDataSize * 10;
		void* framePoolData = malloc(framePoolSize);

		TickStruct data
		{
			window,
			*context,
			input,
			audio,
			resources,
			gameData,
			config,
			jobSys,
			keepRunning,
			{ framePoolData, framePoolSize, config.frameDataSize },
#ifdef EMSCRIPTEN
			pthread_self(),
#endif

#ifdef TAKO_EDITOR
			watcher
#endif
		};

		

#ifndef TAKO_EMSCRIPTEN
		jobSys.Schedule(std::bind(Tick, &data));
		jobSys.JoinAsWorker();
#else
		emscripten_push_main_loop_blocker(EmscriptenDelayed, &data);
		emscripten_set_main_loop_arg(ScheduleTick, &data, 0, 1);
#endif

		free(framePoolData);
		free(gameData);

		LOG("terminating");
		return 0;
	}
}
