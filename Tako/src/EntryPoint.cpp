#include "EntryPoint.hpp"
#include "Tako.hpp"
#include "World.hpp"
#include <emscripten.h>

namespace tako
{
    struct TickStruct
    {
        tako::Window& window;
        tako::GraphicsContext& context;
        tako::PixelArtDrawer* drawer;
        tako::Input& input;
    };

    void Tick(void* p)
    {
        TickStruct* data = reinterpret_cast<TickStruct*>(p);
        data->window.Poll();
        data->input.Update();
        tako::Update(&data->input, 0.16f);
        tako::Draw(data->drawer);
        data->context.Present();
    }

    int RunGameLoop()
    {
        LOG("Init!");
        tako::Window window;
        tako::Input input;
        tako::GraphicsContext context(window.GetHandle(), window.GetWidth(), window.GetHeight());
        auto drawer = context.CreatePixelArtDrawer();
        tako::Setup(drawer);
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
        broadcaster.Register(&input);

        window.SetEventCallback([&](tako::Event& evt)
        {
            broadcaster.Broadcast(evt);
        });


        /*
        while (!window.ShouldExit())
        {
            window.Poll();
            input.Update();
            tako::Update(&input, 0.16f);
            tako::Draw(drawer);
            context.Present();
            //Sleep(16);
        }
        */

        TickStruct data
        {
            window,
            context,
            drawer,
            input
        };

        emscripten_set_main_loop_arg(Tick, &data, 0, 1);

        LOG("terminating");
        return 0;
    }
}