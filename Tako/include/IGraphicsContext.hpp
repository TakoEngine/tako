#pragma once
#include "GraphicsAPI.hpp"
#include "Window.hpp"
#include "Event.hpp"
#include "Texture.hpp"
#include "Bitmap.hpp"
#include "VertexBuffer.hpp"
#include "Material.hpp"
#include "Pipeline.hpp"

namespace tako
{

	struct Vertex
	{
		Vector3 pos;
		Vector3 normal;
		Vector3 color;
		Vector2 uv;

		constexpr bool operator==(const Vertex& other) const
		{
			return
				pos == other.pos &&
				normal == other.normal &&
				color == other.color &&
				uv == other.uv;
		}
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

		virtual void BindPipeline(const Pipeline* pipeline) = 0;
		virtual void BindVertexBuffer(const Buffer* buffer) = 0;
		virtual void BindIndexBuffer(const Buffer* buffer) = 0;
		virtual void BindMaterial(const Material* material) = 0;
		virtual void UpdateUniform(const Matrix4& matrix) = 0;

		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix) = 0;

		virtual Pipeline CreatePipeline(U8* vertCode, size_t vertSize, U8* fragCode, size_t fragSize) = 0;
		virtual Material CreateMaterial(const Texture* texture) = 0;
		virtual Texture CreateTexture(const Bitmap& bitmap) = 0;
		virtual Buffer CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize) = 0;
		
	};
}
