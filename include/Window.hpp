#pragma once
#include <memory>

namespace tako
{
    class Window
    {
    public:
        Window();
        ~Window();
        void Poll();
        bool ShouldExit();
    private:
        class Impl;
        std::unique_ptr<Impl> m_impl;
    };
}