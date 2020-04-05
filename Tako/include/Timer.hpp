#pragma once

namespace tako
{
    class Timer
    {
    public:
        Timer();
        float GetDeltaTime();
    private:
        float m_lastFrame;
    };
}