#pragma once
#include "GraphicsAPI.hpp"
#include "Texture.hpp"
#include "VertexBuffer.hpp"
#include "Material.hpp"
#include "Pipeline.hpp"
#include <cstddef>
#include <span>

import Tako.Math;
import Tako.Bitmap;
import Tako.Event;
import Tako.StringView;
import Tako.Window;

namespace tako
{
	enum class PipelineVectorAttribute
	{
		Vec2,
		Vec3,
		Vec4,
		RGBA
	};

	enum class TextureType
	{
		E2D,
		Cube
	};

	struct PipelineDescriptor
	{
		const char* shaderCode = nullptr;
		const char* vertEntry = nullptr;
		const char* fragEntry = nullptr;
		/*
		U8* vertCode;
		size_t vertSize;
		U8* fragCode;
		size_t fragSize;
		*/

		PipelineVectorAttribute* vertexAttributes = nullptr;
		size_t vertexAttributeSize = 0;

		size_t pipelineUniformSize = 0;

		TextureType samplerTextureType = TextureType::E2D;

		bool usePerDrawModel = true;

		size_t* pushConstants = nullptr;
		size_t pushConstantsSize = 0;

	};

	struct CameraUniformData
	{
		Matrix4 view;
		Matrix4 proj;
		//Matrix4 viewProj;
	};

	struct MaterialDescriptor
	{
		TextureType textureType = TextureType::E2D;
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

		virtual void Draw(U32 vertexCount) = 0;

		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix) = 0;

		virtual Pipeline CreatePipeline(const PipelineDescriptor& pipelineDescriptor) = 0;
		virtual Material CreateMaterial(const Texture texture, const MaterialDescriptor& materialDescriptor = {}) = 0;
		virtual Texture CreateTexture(const ImageView image) = 0;
		virtual Texture CreateTexture(const std::span<const ImageView> images) = 0;
		virtual Buffer CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize) = 0;

		virtual void ReleaseBuffer(Buffer buffer) = 0;

	};
}
