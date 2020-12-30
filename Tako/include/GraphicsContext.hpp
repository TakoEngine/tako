#pragma once
#include "GraphicsAPI.hpp"
#include "Window.hpp"
#include <memory>
#include "Event.hpp"
#include "PixelArtDrawer.hpp"

namespace tako
{
	class GraphicsContext : public IEventHandler
	{
	public:
		static std::unique_ptr<GraphicsContext> Create(Window* window, GraphicsAPI api);
		virtual ~GraphicsContext() {};
		virtual void Present() = 0;
		virtual void Resize(int width, int height) = 0;
		virtual void HandleEvent(Event& evt) override = 0;
		virtual PixelArtDrawer* CreatePixelArtDrawer()  = 0;
	};
}
