#pragma once
#include "GraphicsAPI.hpp"
#include "Window.hpp"
#include "Event.hpp"
#include "Texture.hpp"
#include "Bitmap.hpp"

namespace tako {
	class IGraphicsContext : public IEventHandler
	{
	public:
		virtual ~IGraphicsContext() {};
		virtual void Present() = 0;
		virtual void Resize(int width, int height) = 0;
		virtual void HandleEvent(Event &evt) override = 0;

		virtual Texture CreateTexture(const Bitmap& bitmap) = 0;
	};
}
