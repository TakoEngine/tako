#include "Timer.hpp"
#include "emscripten.h"

namespace tako
{
	Timer::Timer()
	{
		m_lastFrame = emscripten_get_now();
	}

	float Timer::GetDeltaTime()
	{
		double time = emscripten_get_now();
		float dt = (time - m_lastFrame) / 1000;
		m_lastFrame = time;
		return dt;
	}
}
