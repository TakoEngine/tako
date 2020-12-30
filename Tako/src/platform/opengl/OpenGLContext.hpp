#pragma once
#include "GraphicsContext.hpp"
#include "OpenGLPixelArtDrawer.hpp"

namespace tako
{
    class OpenGLContext final : public GraphicsContext
    {
    public:
        OpenGLContext(Window* window);
        ~OpenGLContext() override = default;
        virtual void Present() override;
        virtual void Resize(int width, int height) override;
        virtual void HandleEvent(Event& evt) override;
        virtual PixelArtDrawer* CreatePixelArtDrawer() override;

        OpenGLPixelArtDrawer* GetPixelArtDrawer();
    private:
        WindowHandle m_handle;
        OpenGLPixelArtDrawer* m_drawer = nullptr;
    };
}

