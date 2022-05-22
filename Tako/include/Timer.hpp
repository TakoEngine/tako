#pragma once
#include <atomic>

namespace tako
{
	class Timer
	{
	public:
		Timer();
		float GetDeltaTime();
	private:
		std::atomic<float> m_lastFrame;
	};
}
