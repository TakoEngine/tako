#pragma once
#include "WindowHandle.hpp"
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
        int GetWidth();
        int GetHeight();
        WindowHandle GetHandle() const;
    private:
        class WindowImpl;
        std::unique_ptr<WindowImpl> m_impl;
    };
}