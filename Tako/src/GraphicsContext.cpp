#include "GraphicsContext.hpp"

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

