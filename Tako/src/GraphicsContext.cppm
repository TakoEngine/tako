module;
#include "IGraphicsContext.hpp"
#include "Utility.hpp"
#ifdef TAKO_OPENGL
#include "OpenGLContext.hpp"
#endif
#if TAKO_VULKAN
#include "VulkanContext.hpp"
#endif
#include <memory>
#include <type_traits>
export module Tako.GraphicsContext;

#ifdef TAKO_WEBGPU
export import Tako.WebGPU;
#endif

export namespace tako
{
	constexpr GraphicsAPI SupportedAPIs[] =
	{
#ifdef TAKO_OPENGL
		GraphicsAPI::OpenGL,
#endif
#ifdef TAKO_WEBGPU
		GraphicsAPI::WebGPU,
#endif
#ifdef TAKO_VULKAN
		GraphicsAPI::Vulkan,
#endif
	};

	constexpr size_t SupportedAPICount = sizeof(SupportedAPIs) / sizeof(GraphicsAPI);
	constexpr bool SingleAPI = SupportedAPICount == 1;

	template<GraphicsAPI> struct APITypeMap;

#ifdef TAKO_OPENGL
	template<> struct APITypeMap<GraphicsAPI::OpenGL> {
		using type = OpenGLContext;
	};
#endif
#ifdef TAKO_WEBGPU
	template<> struct APITypeMap<GraphicsAPI::WebGPU> {
		using type = WebGPUContext;
	};
#endif
#ifdef TAKO_VULKAN
	template<> struct APITypeMap<GraphicsAPI::Vulkan> {
		using type = VulkanContext;
	};
#endif

	using GraphicsContext = std::conditional<!SingleAPI, IGraphicsContext, APITypeMap<SupportedAPIs[0]>::type>::type;

	std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window *window, GraphicsAPI api);
	GraphicsAPI ResolveGraphicsAPI(GraphicsAPI api);
}


namespace tako
{
	std::unique_ptr<GraphicsContext> CreateGraphicsContext(Window *window, GraphicsAPI api)
	{
		ASSERT(api != GraphicsAPI::Default);
		if constexpr (SingleAPI)
		{
			ASSERT(api == SupportedAPIs[0]);
			return std::make_unique<APITypeMap<SupportedAPIs[0]>::type>(window);
		}
		switch (api)
		{
#ifdef TAKO_OPENGL
			case GraphicsAPI::OpenGL:
				return std::make_unique<OpenGLContext>(window);
#endif
#ifdef TAKO_WEBGPU
			case GraphicsAPI::WebGPU:
				return std::make_unique<WebGPUContext>(window);
#endif
#ifdef TAKO_VULKAN
			case GraphicsAPI::Vulkan:
				return std::make_unique<VulkanContext>(window);
#endif
			default: return nullptr;
		}
	}

	GraphicsAPI ResolveGraphicsAPI(GraphicsAPI api)
	{
		if (api == GraphicsAPI::Default)
		{
			ASSERT(SupportedAPICount > 0);
			return SupportedAPIs[SupportedAPICount - 1];
		}
		return api;
	}
}
