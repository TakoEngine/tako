#pragma once
#include "GraphicsAPI.hpp"
#include "Window.hpp"
#include "Event.hpp"
#include "Texture.hpp"
#include "Bitmap.hpp"
#include "VertexBuffer.hpp"

namespace tako {

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

		virtual void DrawMesh(const Matrix4& model) = 0;

		virtual Texture CreateTexture(const Bitmap& bitmap) = 0;
		virtual Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices) = 0;
	};
}
