#pragma once
#include "GraphicsAPI.hpp"
#include "Window.hpp"
#include "Event.hpp"
#include "Texture.hpp"
#include "Bitmap.hpp"
#include "VertexBuffer.hpp"

namespace tako
{

	struct Vertex
	{
		Vector3 pos;
		Vector3 color;
	};

	class IGraphicsContext : public IEventHandler
	{
	public:
		virtual ~IGraphicsContext() {};
		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void Present() = 0;
		virtual void Resize(int width, int height) = 0;
		virtual void HandleEvent(Event &evt) override = 0;

		virtual void BindVertexBuffer(const Buffer* buffer) = 0;
		virtual void BindIndexBuffer(const Buffer* buffer) = 0;
		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix) = 0;

		virtual Texture CreateTexture(const Bitmap& bitmap) = 0;
		virtual Buffer CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize) = 0;
	};
}
