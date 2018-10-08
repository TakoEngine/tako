#pragma once
#include "Window.hpp"
#include <memory>

namespace tako
{
    class GraphicsContext
    {
    public:
        GraphicsContext(Window& window);
        ~GraphicsContext();
        void Present();
    private:
        class ContextImpl;
        std::unique_ptr<ContextImpl> m_impl;
    };
}