#pragma once
#include "GraphicsAPI.hpp"
#include "Texture.hpp"
#include "VertexBuffer.hpp"
#include "Pipeline.hpp"
#include <cstddef>
#include <variant>
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

	enum class ShaderBindingType
	{
		Uniform,
		Storage,
		Texture2D,
		TextureCube,
		Sampler
	};

	struct ShaderEntry
	{
		ShaderBindingType type;
		size_t size;
	};

	struct ShaderBindingDescriptor
	{
		std::span<ShaderEntry> entries;
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

		std::span<ShaderBindingDescriptor> shaderBindings;

		bool usePerDrawModel = true;
	};

	using ShaderBindingEntryData = std::variant<Buffer, Texture, Sampler>;

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
		virtual void Bind(ShaderBinding binding, U32 slot) = 0;

		virtual void Draw(U32 vertexCount) = 0;

		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix) = 0;

		virtual Pipeline CreatePipeline(const PipelineDescriptor& pipelineDescriptor) = 0;
		virtual ShaderBindingLayout GetPipelineShaderBindingLayout(Pipeline pipeline, size_t slot) = 0;
		virtual Texture CreateTexture(const ImageView image, TextureType type = TextureType::E2D) = 0;
		virtual Texture CreateTexture(std::span<const ImageView> images, TextureType type = TextureType::Cube) = 0;
		virtual void ReleaseTexture(Texture texture) = 0;

		virtual Sampler CreateSampler() = 0;

		virtual Buffer CreateBuffer(BufferType bufferType, size_t bufferSize) = 0;
		virtual Buffer CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize) = 0;
		virtual void UpdateBuffer(Buffer buffer, const void* data, size_t writeSize, size_t offset = 0) = 0;
		virtual void ReleaseBuffer(Buffer buffer) = 0;

		virtual ShaderBinding CreateShaderBinding(ShaderBindingLayout layout, std::span<ShaderBindingEntryData> entryData) = 0;
		virtual void ReleaseShaderBinding(ShaderBinding binding) = 0;
	};
}
