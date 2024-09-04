#define TAKO_FORCE_LOG
#include "Utility.hpp"
#include <chrono>

import Tako.World;
import Tako.Math;

struct Position
{
	tako::Vector2 pos;
};

struct Velocity
{
	tako::Vector2 vel;
};

class Timer
{
public:
	Timer()
	{
		Start();
	}

	void Start()
	{
		m_start = std::chrono::high_resolution_clock::now();
	}

	double Stop()
	{
		auto endTime = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
		auto duration = end - start;
		return duration * 0.001;
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};

constexpr auto REPEAT_COUNT = 10000;
constexpr auto COMP_COUNT = 10000000;

template<typename Cb>
void RunTimed(std::string_view name, Cb callback)
{
	double timeSum = 0;
	Timer timer;
	for (int i = 0; i < REPEAT_COUNT; i++)
	{
		timer.Start();
		callback([&]{ timer.Start(); });
		timeSum += timer.Stop();
	}

	LOG("{}: {}", name, timeSum / REPEAT_COUNT);
}

int main()
{
	tako::World world;
	for (int i = 0; i < COMP_COUNT; i++)
	{
		world.Create<Position>({tako::Vector2(i, i)});
	}


	float sum = std::numeric_limits<float>::min();
	RunTimed("IterateComp", [&](auto start)
	{
		world.IterateComp<Position>([&](Position& pos)
		{
			sum += pos.pos.x;
		});
	});

	RunTimed("IterateComps", [&](auto start)
	{
		world.IterateComps<Position>([&](Position& pos)
		{
			sum += pos.pos.x;
		});
	});

	LOG("{}", sum);
}
