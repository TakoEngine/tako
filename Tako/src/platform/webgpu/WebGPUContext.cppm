module;
#include "Utility.hpp"
#include <IGraphicsContext.hpp>
#include <webgpu/webgpu.h>
export module Tako.WebGPU;

namespace tako
{
	export class WebGPUContext final : public IGraphicsContext
	{
	public:
		WebGPUContext(Window* window)
		{
			WGPUInstanceDescriptor desc = {};
			desc.nextInChain = nullptr;

			#ifdef EMSCRIPTEN
				m_instance = wgpuCreateInstance(nullptr);
			#else
				m_instance = wgpuCreateInstance(&desc);
			#endif
			ASSERT(m_instance);
		}

		virtual ~WebGPUContext() override
		{
			wgpuInstanceRelease(m_instance);
		}


		virtual void Begin()
		{

		}

		virtual void End()
		{

		}

		virtual void Present()
		{

		}

		virtual void Resize(int width, int height)
		{

		}

		virtual void HandleEvent(Event &evt)
		{

		}


		virtual U32 GetWidth()
		{
			return 0;
		}

		virtual U32 GetHeight()
		{
			return 0;
		}


		virtual void BindPipeline(const Pipeline* pipeline)
		{

		}

		virtual void BindVertexBuffer(const Buffer* buffer)
		{

		}

		virtual void BindIndexBuffer(const Buffer* buffer)
		{

		}

		virtual void BindMaterial(const Material* material)
		{

		}


		virtual void UpdateCamera(const CameraUniformData& cameraData)
		{

		}

		virtual void UpdateUniform(const void* uniformData, size_t uniformSize)
		{

		}


		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix)
		{

		}

		virtual void DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix)
		{

		}


		virtual Pipeline CreatePipeline(const PipelineDescriptor& pipelineDescriptor)
		{
			return {};
		}

		virtual Material CreateMaterial(const Texture* texture)
		{
			return {};
		}

		virtual Texture CreateTexture(const Bitmap& bitmap)
		{
			return {};
		}

		virtual Buffer CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize)
		{
			return {};
		}
	private:
		WGPUInstance m_instance;
	};
}
