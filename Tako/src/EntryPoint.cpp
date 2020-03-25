#include "EntryPoint.hpp"
#include "Tako.hpp"
#include "World.hpp"

namespace tako
{
    int RunGameLoop()
    {
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


        while (!window.ShouldExit())
        {
            window.Poll();
            input.Update();
            tako::Update(&input, 0.16f);
            tako::Draw(drawer);
            context.Present();
            //Sleep(16);
        }

        LOG("terminating")
        return 0;
    }
}