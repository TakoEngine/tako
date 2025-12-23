module;
#include "Tako.hpp"
#include "Timer.hpp"
#include <thread>
#include <functional>
//#include "OpenGLPixelArtDrawer.hpp"
#ifdef TAKO_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/proxying.h>
#endif
#ifdef TAKO_EDITOR
#include "FileWatcher.hpp"
#endif

#ifdef TAKO_IMGUI
#include "imgui.h"

#if defined(TAKO_GLFW)
#include "imgui_impl_glfw.h"
#elif defined(TAKO_WIN32)
#include "imgui_impl_win32.h"
#endif
#ifdef TAKO_OPENGL
#include "imgui_impl_opengl3.h"
#endif
#ifdef TAKO_WEBGPU
#include "imgui_impl_wgpu.h"
#endif
#endif

export module EntryPoint;

import Tako.InputEvent;
import Tako.Audio;
//import Tako.Renderer3D;
import Tako.Application;
//import Tako.Serialization;
import Tako.VFS;
import Tako.Resources;
import Tako.JobSystem;
import Tako.RmlUi;
import Tako.Allocators.CachePoolAllocator;
#ifdef TAKO_IMGUI
import Tako.ImGui;
#endif

namespace tako
{
	struct TickStruct
	{
		tako::Window& window;
		tako::GraphicsContext& context;
		tako::Input& input;
		tako::Audio& audio;
        tako::VFS& vfs;
		tako::Resources& resources;
		tako::RmlUi& ui;
		void* gameData;
		GameConfig& config;
		JobSystem& jobSys;
		std::atomic<bool>& keepRunning;
		//Allocators::PoolAllocator frameDataPool;
		Allocators::CachePoolAllocator frameDataAllocator;
#ifdef TAKO_EDITOR
		tako::FileWatcher& watcher;
#endif
#ifdef TAKO_EMSCRIPTEN
		pthread_t mainThread;
		emscripten::ProxyingQueue proxyQueue;
#endif

		std::atomic_flag frameDataPoolLock = ATOMIC_FLAG_INIT;
		std::atomic<int> frame = 0;
		int openFrames = 1;
		std::atomic_flag frameCounterLock = ATOMIC_FLAG_INIT;

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
#ifdef TAKO_EDITOR
		for (auto& change: data->watcher.Poll())
		{
			auto path = std::filesystem::relative(change.path, change.mountPath).string();
			//TODO: make more robust
			for (auto& c : path)
			{
				if (c == '\\')
				{
					c = '/';
				}
			}
			path = "/" + path;
			LOG("Filechange detected {}", path);
			//TODO: Check if change is the highest priority mount path
			data->resources.Reload(path);
		}
#endif

#ifdef TAKO_GLFW
		data->jobSys.ScheduleForThread(0, [=]()
#else
		data->jobSys.Schedule([=]()
#endif
		{
			data->window.Poll();
			data->input.Update();
#ifdef TAKO_IMGUI
			switch (data->context.GetAPI())
			{
				#ifdef TAKO_OPENGL
				case GraphicsAPI::OpenGL:
					ImGui_ImplOpenGL3_Init();
					break;
				#endif
				#ifdef TAKO_WEBGPU
				case GraphicsAPI::WebGPU:
					ImGui_ImplWGPU_NewFrame();
					break;
				#endif
			}

#if defined(TAKO_GLFW)
			ImGui_ImplGlfw_NewFrame();
#elif defined(TAKO_WIN32)
			ImGui_ImplWin32_NewFrame();
#endif
			ImGui::NewFrame();
#endif
		});


		if (data->config.CheckFrameDataSizeChange)
		{
			data->config.CheckFrameDataSizeChange(data->gameData, data->config.frameDataSize);
		}
		auto frameDataSize = data->config.frameDataSize;
		/*
		while (data->frameDataPoolLock.test_and_set(std::memory_order_acquire));
		void* frameData = data->frameDataAllocator.Allocate(frameDataSize);
		data->frameDataPoolLock.clear(std::memory_order_release);
		*/
		void* frameData = malloc(frameDataSize);

		data->jobSys.Continuation([=]()
		{
			if (data->config.Update)
			{
				data->jobSys.Schedule([=]()
				{
					GameStageData stageData
					{
						data->gameData,
						frameData,
						frameDataSize
					};
					if (data->config.Update)
					{
						data->config.Update(stageData, &data->input, dt);
					}
					data->ui.Update();
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
				#ifdef TAKO_EMSCRIPTEN
				//data->proxyQueue.proxySync(data->mainThread, [=]()
				#endif
				{
					data->context.Begin();
					if (data->config.Draw)
					{
						GameStageData stageData
						{
							data->gameData,
							frameData,
							frameDataSize
						};
						data->config.Draw(stageData);
					}
					data->ui.Draw();
					#ifdef TAKO_IMGUI
					ImGui::Render();
					switch (data->context.GetAPI())
					{
						#ifdef TAKO_OPENGL
						case GraphicsAPI::OpenGL:
							ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
							break;
						#endif
						#ifdef TAKO_WEBGPU
						case GraphicsAPI::WebGPU:
							WebGPUContext* wcontext = reinterpret_cast<WebGPUContext*>(&data->context);
							ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), wcontext->GetRenderPass());
							break;
						#endif
					}
					#endif
					data->context.End();

					#ifdef TAKO_IMGUI
					if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
					{
						ImGui::UpdatePlatformWindows();
						ImGui::RenderPlatformWindowsDefault();
					}
					#endif
					data->context.Present();
					//LOG("Tick End");
				#ifdef TAKO_EMSCRIPTEN
				}
				#else
				}
				#endif

				#ifndef EMSCRIPTEN
					data->jobSys.ScheduleDetached(std::bind(Tick, p));
				#endif

				data->jobSys.Continuation([=]()
				{
					/*
					while (data->frameDataPoolLock.test_and_set(std::memory_order_acquire));
					data->frameDataAllocator.Deallocate(frameData, frameDataSize);
					data->frameDataPoolLock.clear(std::memory_order_release);
					*/
					free(frameData);
					//LOG("Tick End");
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
				data->config.Setup(data->gameData, { &data->context, &data->vfs, &data->resources, &data->audio, &data->ui });
			});
		}
	}

	void ScheduleTick(void* p)
	{
		//LOG("Schedule Tick")
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

			#if defined(TAKO_GLFW)
				#ifdef TAKO_OPENGL
					ImGui_ImplGlfw_InitForOpenGL(window.GetHandle(), true);
				#else
					ImGui_ImplGlfw_InitForOther(window.GetHandle(), true);
				#endif
				#ifdef TAKO_EMSCRIPTEN
					ImGui_ImplGlfw_InstallEmscriptenCallbacks(window.GetHandle(), "#canvas");
				#endif
			#elif defined(TAKO_WIN32)
				ImGui_ImplWin32_Init(window.GetHandle());
			#endif
			switch (api)
			{
				#ifdef TAKO_OPENGL
				case GraphicsAPI::OpenGL:
					ImGui_ImplOpenGL3_Init();
					break;
				#endif
				#ifdef TAKO_WEBGPU
				case GraphicsAPI::WebGPU:
					WebGPUContext* wcontext = reinterpret_cast<WebGPUContext*>(context.get());
					ImGui_ImplWGPU_InitInfo init_info;
					init_info.Device = wcontext->GetDevice();
					init_info.NumFramesInFlight = 3;
					init_info.RenderTargetFormat = wcontext->GetSurfaceFormat();
					init_info.DepthStencilFormat = wcontext->GetDepthTextureFormat();
					ImGui_ImplWGPU_Init(&init_info);
					break;
				#endif
			}

		#endif
		tako::Broadcaster broadcaster;
		tako::InputBroadcaster inputBroadcaster;

		VFS vfs;
		#ifdef TAKO_EMSCRIPTEN
			vfs.AddMountPath("");
		#else
			vfs.AddMountPath("./Assets");
		#endif
		Resources resources(&vfs);
		RmlUi ui;
		ui.Init(&window, context.get(), &vfs, &resources);

		void* gameData = malloc(config.gameDataSize);
#ifndef EMSCRIPTEN
		if (config.Setup)
		{
			config.Setup(gameData, { context.get(), &vfs, &resources, &audio, &ui });
		}
#endif

#ifdef TAKO_EDITOR
		tako::FileWatcher watcher(&vfs);
#endif

		std::atomic<bool> keepRunning = true;
		tako::CallbackEventHandler onEvent([&](tako::Event& ev)
		{
			//LOG("Event: {}", ev);
			switch (ev.GetType())
			{
				case tako::EventType::FramebufferResize:
				{
					tako::FramebufferResize& res = static_cast<tako::FramebufferResize&>(ev);
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
		#ifdef TAKO_IMGUI
			ImGuiInputHandler imguiInput;
			inputBroadcaster.Register(&imguiInput);
		#endif
		inputBroadcaster.Register(&ui);
		inputBroadcaster.Register(&input);

		window.SetEventCallback([&](tako::Event& evt)
		{
			broadcaster.Broadcast(evt);
		});
		window.SetInputCallback([&](tako::InputEvent& evt)
		{
			return inputBroadcaster.Broadcast(evt);
		});

		//size_t framePoolSize = config.frameDataSize * 10;
		//void* framePoolData = malloc(framePoolSize);

		TickStruct data
		{
			window,
			*context,
			input,
			audio,
			vfs,
			resources,
			ui,
			gameData,
			config,
			jobSys,
			keepRunning,
			//{ framePoolData, framePoolSize, config.frameDataSize },
			{},
#ifdef TAKO_EDITOR
			watcher
#endif
#ifdef EMSCRIPTEN
			pthread_self(),
#endif
		};


#ifndef TAKO_EMSCRIPTEN
		jobSys.Schedule(std::bind(Tick, &data));
		jobSys.JoinAsWorker();
#else
		emscripten_push_main_loop_blocker(EmscriptenDelayed, &data);
		emscripten_set_main_loop_arg(ScheduleTick, &data, 0, 1);
#endif

		//free(framePoolData);
		free(gameData);

		LOG("terminating");
		return 0;
	}
}
