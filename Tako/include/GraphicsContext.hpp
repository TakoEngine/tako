#pragma once
#include "IGraphicsContext.hpp"
#include <memory>
#include <type_traits>
#ifdef TAKO_OPENGL
#include "OpenGLContext.hpp"
#endif
#if TAKO_VULKAN
#include "VulkanContext.hpp"
#endif

namespace tako
{
    constexpr GraphicsAPI SupportedAPIs[] =
    {
#ifdef TAKO_OPENGL
        GraphicsAPI::OpenGL,
#endif
#ifdef TAKO_VULKAN
        GraphicsAPI::Vulkan,
#endif
    };

    constexpr int SingleAPI = sizeof(SupportedAPIs) / sizeof(GraphicsAPI) == 1;

    template<GraphicsAPI> struct APITypeMap;

#ifdef TAKO_OPENGL
    template<> struct APITypeMap<GraphicsAPI::OpenGL> {
        using type = OpenGLContext;
    };
#endif
#ifdef TAKO_VULKAN
    template<> struct APITypeMap<GraphicsAPI::Vulkan> {
        using type = VulkanContext;
    };
#endif

    using GraphicsContext = std::conditional<!SingleAPI, IGraphicsContext, APITypeMap<SupportedAPIs[0]>::type>::type;

    std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window *window, GraphicsAPI api);
}
