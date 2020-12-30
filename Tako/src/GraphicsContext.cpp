#include "GraphicsContext.hpp"

#ifdef TAKO_OPENGL
#include "OpenGLContext.hpp"
#endif
#if TAKO_VULKAN
#include "VulkanContext.hpp"
#endif

namespace tako
{
    std::unique_ptr<GraphicsContext> GraphicsContext::Create(Window *window, GraphicsAPI api)
    {
        switch (api)
        {
#ifdef TAKO_OPENGL
            case GraphicsAPI::OpenGL:
                return std::make_unique<OpenGLContext>(window);
#endif
#ifdef TAKO_VULKAN
            case GraphicsAPI::Vulkan:
                return std::make_unique<VulkanContext>(window);
#endif
            default: return nullptr;
        }
    }
}

