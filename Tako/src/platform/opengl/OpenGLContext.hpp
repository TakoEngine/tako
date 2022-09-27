#pragma once
#include "IGraphicsContext.hpp"

namespace tako
{
	class OpenGLContext final : public IGraphicsContext
	{
	public:
		OpenGLContext(Window* window);
		~OpenGLContext() override = default;
		virtual void Begin() override;
		virtual void End() override;
		virtual void Present() override;
		virtual void Resize(int width, int height) override;
		virtual void HandleEvent(Event& evt) override;

		virtual U32 GetWidth() override;
		virtual U32 GetHeight() override;
		
		virtual void BindPipeline(const Pipeline* pipeline) override;
		virtual void BindVertexBuffer(const Buffer* buffer) override;
		virtual void BindIndexBuffer(const Buffer* buffer) override;
		virtual void BindMaterial(const Material* material) override;

		virtual void UpdateCamera(const CameraUniformData& cameraData) override;
		virtual void UpdateUniform(const void* uniformData, size_t uniformSize) override;

		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix) override;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix) override;

		virtual Pipeline CreatePipeline(const PipelineDescriptor& pipelineDescriptor) override;
		virtual Material CreateMaterial(const Texture* texture) override;
		virtual Texture CreateTexture(const Bitmap& bitmap) override;
		virtual Buffer CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize) override;
	private:
		WindowHandle m_handle;
		U32 m_width, m_height;
	};
}

