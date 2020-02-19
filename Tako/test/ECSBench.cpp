#define TAKO_FORCE_LOG
#include "World.hpp"
#include "Math.hpp"
#include <chrono>

struct Position
{
    tako::Vector2 pos;
};

struct Velocity
{
    tako::Vector2 vel;
};

int main()
{
    tako::World world;
    LOG("{}", world.Create().id);
    LOG("{}", world.Create<Velocity>().id);
    LOG("{}", world.Create<Position>().id);
    LOG("{}", world.Create<Position, Velocity>().id);
    LOG("{}", world.Create<Position>().id);

    for (int i = 0; i < 10000000; i++)
    {
        world.Create<Position, Velocity>();
    }


    auto entity = world.Create<Position>();
    auto& pos = world.GetComponent<Position>(entity);
    pos.pos = { 4, 2 };

    //LOG("{}", world.Create<Position, tako::Window>().id);

    LOG("Iter Handle");
    auto t1 = std::chrono::high_resolution_clock::now();
    world.IterateHandle<Position, Velocity>([&](tako::EntityHandle handle) {
        Position &pos = world.GetComponent<Position>(handle);
        //LOG("Iter id: {} x: {} y: {}", handle.id, pos.pos.x, pos.pos.y);
        pos.pos.x++;
        Velocity &vel = world.GetComponent<Velocity>(handle);
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
    for (auto [pos, vel] : world.Iter<Position, Velocity>())
    {
        //LOG("Iter x: {} y: {}", pos.pos.x, pos.pos.y);
        pos.pos.x++;
        vel.vel.x++;
    }
    t2 = std::chrono::high_resolution_clock::now();
    LOG("Ticks2: {}", (t2 - t1).count());

    int sum = 0;
    for (auto [pos] : world.Iter<Position>())
    {
        sum += pos.pos.x;
    }
    LOG("Sum: {}", sum);

    LOG("Iter Comps:");
    t1 = std::chrono::high_resolution_clock::now();
    world.IterateComps<Position, Velocity>([&](Position& pos, Velocity& vel)
    {
        //auto [pos] = tup;
        //LOG("Iter x: {} y: {}", pos.pos.x, pos.pos.y);
        pos.pos.x++;
        vel.vel.x++;
    });
    t2 = std::chrono::high_resolution_clock::now();
    LOG("Ticks3: {}", (t2 - t1).count());

    sum = 0;
    for (auto [pos] : world.Iter<Position>())
    {
        sum += pos.pos.x;
    }
    LOG("Sum: {}", sum);

    LOG("Iter Comp 2:");
    t1 = std::chrono::high_resolution_clock::now();
    world.IterateComp<Position>([&](Position& pos)
    {
        //LOG("Iter x: {} y: {}", pos.pos.x, pos.pos.y);
        pos.pos.x++;
    });
    t2 = std::chrono::high_resolution_clock::now();
    LOG("Ticks4: {}", (t2 - t1).count());

    return 0;
}