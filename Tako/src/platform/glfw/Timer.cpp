#include "Timer.hpp"
#include "GLFW/glfw3.h"

namespace tako
{
    Timer::Timer()
    {
        m_lastFrame = glfwGetTime();
    }

    float Timer::GetDeltaTime()
    {
        double time = glfwGetTime();
        float dt = time - m_lastFrame;
        m_lastFrame = time;
        return dt;
    }
}