#pragma once
#include "IGraphicsContext.hpp"

namespace tako
{
	class OpenGLPixelArtDrawer;
	class OpenGLContext final : public IGraphicsContext
	{
	public:
		OpenGLContext(Window* window);
		~OpenGLContext() override = default;
		virtual void Present() override;
		virtual void Resize(int width, int height) override;
		virtual void HandleEvent(Event& evt) override;

		virtual Texture CreateTexture(const Bitmap& bitmap) override;
	private:
		WindowHandle m_handle;
	};
}

