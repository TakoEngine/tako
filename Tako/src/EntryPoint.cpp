#include "EntryPoint.hpp"
#include "Tako.hpp"
#include "World.hpp"
#include "Timer.hpp"
#include "Resources.hpp"
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
        auto api = tako::GraphicsAPI::Vulkan;
        tako::Window window(api);
        tako::Input input;
        auto context = tako::GraphicsContext::Create(&window, api);
        auto drawer = context->CreatePixelArtDrawer();
        Audio audio;
        audio.Init();
        Resources resources(drawer);
        tako::Setup(drawer, &resources);
        tako::Broadcaster broadcaster;
#ifdef TAKO_EDITOR
        tako::FileWatcher watcher("./Assets");
#endif

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