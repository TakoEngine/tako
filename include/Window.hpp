#pragma once
#include <memory>
#include "Bitmap.hpp"

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
        void DrawBitmap(const Bitmap& bitmap);
    private:
        class WindowImpl;
        std::unique_ptr<WindowImpl> m_impl;
    };
}