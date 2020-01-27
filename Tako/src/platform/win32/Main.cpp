#include "Tako.hpp"
#include "World.hpp"
#include "Windows.h"
#include <chrono>

struct Position
{
	tako::Vector2 pos;
};

struct Velocity
{
	tako::Vector2 vel;
};

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
#ifdef NDEBUG
	{
		AllocConsole();
		HWND hWnd = GetConsoleWindow();
		ShowWindow(hWnd, SW_SHOWNORMAL);
	}
#endif
	//tako::Archetype::Create<tako::Window, tako::Bitmap>();
	tako::World world;
	LOG("{}", world.Create().id);
	LOG("{}", world.Create<Velocity>().id);
	LOG("{}", world.Create<Position>().id);
	LOG("{}", world.Create<Position, Velocity>().id);
	LOG("{}", world.Create<Position>().id);

	for (int i = 0; i < 1000000000; i++)
	{
		world.Create<Position, Velocity>();
	}
	

	auto entity = world.Create<Position>();
	auto& pos = world.GetComponent<Position>(entity);
	pos.pos = { 4, 2 };

	LOG("{}", world.Create<Position, tako::Window>().id);

	LOG("Iter Handle");
	auto t1 = std::chrono::high_resolution_clock::now();
	world.Iterate<Position, Velocity>([&](tako::EntityHandle handle)
	{
		Position& pos = world.GetComponent<Position>(handle);
		//LOG("Iter id: {} x: {} y: {}", handle.id, pos.pos.x, pos.pos.y);
		pos.pos.x++;
		Velocity& vel = world.GetComponent<Velocity>(handle);
		vel.vel.x++;
	});
	auto t2 = std::chrono::high_resolution_clock::now();
	LOG("Ticks: {}", (t2 - t1).count());

	LOG("Iter Comp:");
	t1 = std::chrono::high_resolution_clock::now();
	world.IterateComp<Position>([&](Position& pos)
	{
		//LOG("Iter x: {} y: {}", pos.pos.x, pos.pos.y);
		pos.pos.x++;
	});
	t2 = std::chrono::high_resolution_clock::now();
	LOG("Ticks1: {}", (t2 - t1).count());

	LOG("iter created!");
	t1 = std::chrono::high_resolution_clock::now();
	for (auto [pos] : world.Iter<Position>())
	{
		//LOG("Iter x: {} y: {}", pos.pos.x, pos.pos.y);
		pos.pos.x++;
		//vel.vel.x++;
	}
	t2 = std::chrono::high_resolution_clock::now();
	LOG("Ticks2: {}", (t2 - t1).count());
	for (;;) {}
	return 0;
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
