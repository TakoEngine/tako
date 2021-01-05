#include "Timer.hpp"
#include <chrono>

double GetTime()
{
	static auto start = std::chrono::steady_clock::now();
	std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
	return duration.count();
}

namespace tako
{
	Timer::Timer()
	{
		m_lastFrame = GetTime();
	}

	float Timer::GetDeltaTime()
	{
		double time = GetTime();
		float dt = time - m_lastFrame;
		m_lastFrame = time;
		return dt;
	}
}
