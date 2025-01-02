#pragma once
#include "GraphicsAPI.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "VertexBuffer.hpp"
#include "Material.hpp"
#include "Pipeline.hpp"

import Tako.Math;
import Tako.Bitmap;
import Tako.NumberTypes;
import Tako.Event;

namespace tako
{
	enum class PipelineVectorAttribute
	{
		Vec2,
		Vec3
	};

	struct PipelineDescriptor
	{
		U8* vertCode;
		size_t vertSize;
		U8* fragCode;
		size_t fragSize;

		PipelineVectorAttribute* vertexAttributes;
		size_t vertexAttributeSize;

		size_t pipelineUniformSize;

		size_t* pushConstants;
		size_t pushConstantsSize;

	};

	struct CameraUniformData
	{
		Matrix4 view;
		Matrix4 proj;
		//Matrix4 viewProj;
	};

	class IGraphicsContext : public IEventHandler
	{
	public:
		virtual ~IGraphicsContext() {};
		virtual GraphicsAPI GetAPI() = 0;
		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void Present() = 0;
		virtual void Resize(int width, int height) = 0;
		virtual void HandleEvent(Event &evt) override = 0;

		virtual U32 GetWidth() = 0;
		virtual U32 GetHeight() = 0;

		virtual void BindPipeline(const Pipeline* pipeline) = 0;
		virtual void BindVertexBuffer(const Buffer* buffer) = 0;
		virtual void BindIndexBuffer(const Buffer* buffer) = 0;
		virtual void BindMaterial(const Material* material) = 0;

		virtual void UpdateCamera(const CameraUniformData& cameraData) = 0;
		virtual void UpdateUniform(const void* uniformData, size_t uniformSize, size_t offset = 0) = 0;

		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix) = 0;

		virtual Pipeline CreatePipeline(const PipelineDescriptor& pipelineDescriptor) = 0;
		virtual Material CreateMaterial(const Texture* texture) = 0;
		virtual Texture CreateTexture(const Bitmap& bitmap) = 0;
		virtual Buffer CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize) = 0;

	};
}
