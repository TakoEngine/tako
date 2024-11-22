module;
#include "Utility.hpp"
#include <IGraphicsContext.hpp>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
#include <webgpu/webgpu.h>
export module Tako.WebGPU;

import Tako.JobSystem;

namespace tako
{
	export class WebGPUContext final : public IGraphicsContext
	{
	public:
		WebGPUContext(Window* window) : m_window(window)
		{
			WGPUInstanceDescriptor desc = {};
			desc.nextInChain = nullptr;

			#ifdef EMSCRIPTEN
				WGPUInstance instance = wgpuCreateInstance(nullptr);
			#else
				WGPUInstance instance = wgpuCreateInstance(&desc);
			#endif
			ASSERT(instance);
			m_instance = instance;

			WGPURequestAdapterOptions adapterOpts = {};
			adapterOpts.nextInChain = nullptr;

			wgpuInstanceRequestAdapter(
				instance,
				&adapterOpts,
				[](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* pUserData)
				{
					reinterpret_cast<WebGPUContext*>(pUserData)->RequestAdapterCallback(status, adapter, message);
				},
				this
			);
			//wgpuInstanceRelease(instance);

			while (!m_initComplete)
			{
				emscripten_sleep(100);
			}
		}

		virtual ~WebGPUContext() override
		{
			wgpuQueueRelease(m_queue);
			wgpuDeviceRelease(m_device);
			wgpuAdapterRelease(m_adapter);
			wgpuSurfaceUnconfigure(m_surface);
			wgpuSurfaceRelease(m_surface);
			wgpuInstanceRelease(m_instance);
		}


		virtual void Begin() override
		{
			WGPUTextureView targetView = GetNextSurfaceTextureView();
			ASSERT(targetView);

			WGPUCommandEncoderDescriptor encoderDesc = {};
			encoderDesc.nextInChain = nullptr;
			encoderDesc.label = "DefaultEncoder";
			WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device, &encoderDesc);

			WGPURenderPassDescriptor renderPassDesc = {};
			renderPassDesc.nextInChain = nullptr;

			WGPURenderPassColorAttachment renderPassColorAttachment = {};
			renderPassColorAttachment.view = targetView;
			renderPassColorAttachment.resolveTarget = nullptr;
			renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
			renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
			renderPassColorAttachment.clearValue = WGPUColor{ 0.4, 0.1, 0.8, 1.0 };
			renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED; //Might cause problems with WGPU-native

			renderPassDesc.colorAttachmentCount = 1;
			renderPassDesc.colorAttachments = &renderPassColorAttachment;
			renderPassDesc.depthStencilAttachment = nullptr;
			renderPassDesc.timestampWrites = nullptr;


			WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);


			wgpuRenderPassEncoderEnd(renderPass);
			wgpuRenderPassEncoderRelease(renderPass);

			WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
			cmdBufferDescriptor.nextInChain = nullptr;
			cmdBufferDescriptor.label = "Command buffer";
			WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
			wgpuCommandEncoderRelease(encoder);

			wgpuQueueSubmit(m_queue, 1, &command);
			wgpuCommandBufferRelease(command);

			wgpuTextureViewRelease(targetView);
		}

		virtual void End() override
		{
			//wgpuTextureViewRelease(m_currentTargetView);
			//m_currentTargetView = nullptr;
		}

		virtual void Present() override
		{
			#ifndef EMSCRIPTEN
			wgpuSurfacePresent(surface);
			#endif
		}

		virtual void Resize(int width, int height) override
		{

		}

		virtual void HandleEvent(Event &evt) override
		{

		}


		virtual U32 GetWidth() override
		{
			return 0;
		}

		virtual U32 GetHeight() override
		{
			return 0;
		}


		virtual void BindPipeline(const Pipeline* pipeline) override
		{

		}

		virtual void BindVertexBuffer(const Buffer* buffer) override
		{

		}

		virtual void BindIndexBuffer(const Buffer* buffer) override
		{

		}

		virtual void BindMaterial(const Material* material) override
		{

		}


		virtual void UpdateCamera(const CameraUniformData& cameraData) override
		{

		}

		virtual void UpdateUniform(const void* uniformData, size_t uniformSize) override
		{

		}


		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix) override
		{

		}

		virtual void DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix) override
		{

		}


		virtual Pipeline CreatePipeline(const PipelineDescriptor& pipelineDescriptor) override
		{
			return {};
		}

		virtual Material CreateMaterial(const Texture* texture) override
		{
			return {};
		}

		virtual Texture CreateTexture(const Bitmap& bitmap) override
		{
			return {};
		}

		virtual Buffer CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize) override
		{
			return {};
		}
	private:
		WGPUTextureView m_currentTargetView;

		bool m_initComplete = false; //TODO: tie into "await" system
		Window* m_window;
		WGPUQueue m_queue;
		WGPUDevice m_device;
		WGPUAdapter m_adapter;
		WGPUSurface m_surface;
		WGPUInstance m_instance;

		void RequestAdapterCallback(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message)
		{
			if (status == WGPURequestAdapterStatus_Success)
			{
				m_adapter = adapter;
				WGPUDeviceDescriptor deviceDesc = {};
				deviceDesc.nextInChain = nullptr;
				deviceDesc.label = "DefaultDevice";
				deviceDesc.requiredFeatureCount = 0;
				deviceDesc.requiredLimits = nullptr;
				deviceDesc.defaultQueue.nextInChain = nullptr;
				deviceDesc.defaultQueue.label = "DefaultQueue";
				deviceDesc.deviceLostCallback = DeviceLostCallback;

				wgpuAdapterRequestDevice(
					adapter,
					&deviceDesc,
					[](WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* pUserData)
					{
						reinterpret_cast<WebGPUContext*>(pUserData)->RequestDeviceCallback(status, device, message);
					},
					this
				);
				//wgpuAdapterRelease(adapter);
			}
			else
			{
				LOG_ERR("Error requesting adapter: {}", message);
			}
		}

		void RequestDeviceCallback(WGPURequestDeviceStatus status, WGPUDevice device, char const* message)
		{
			if (status == WGPURequestDeviceStatus_Success)
			{
				m_device = device;
				wgpuDeviceSetUncapturedErrorCallback(m_device, UncapturedErrorCallback, nullptr);

				m_queue = wgpuDeviceGetQueue(m_device);
				//wgpuQueueOnSubmittedWorkDone(m_queue, OnQueueWorkDone, this);

				// Surface
				WGPUSurfaceDescriptorFromCanvasHTMLSelector fromCanvasHTMLSelector;
				fromCanvasHTMLSelector.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
				fromCanvasHTMLSelector.chain.next = NULL;
				fromCanvasHTMLSelector.selector = "#canvas"; //TODO: Use HTML_TARGET from Window

				WGPUSurfaceDescriptor surfaceDescriptor;
				surfaceDescriptor.nextInChain = &fromCanvasHTMLSelector.chain;
				surfaceDescriptor.label = NULL;

				m_surface = wgpuInstanceCreateSurface(m_instance, &surfaceDescriptor);
				ASSERT(m_surface);

				WGPUSurfaceConfiguration config = {};
				config.nextInChain = nullptr;
				config.width = m_window->GetWidth();
				config.height = m_window->GetHeight();

				WGPUTextureFormat surfaceFormat = wgpuSurfaceGetPreferredFormat(m_surface, m_adapter);
				config.format = surfaceFormat;
				config.viewFormatCount = 0;
				config.viewFormats = nullptr;
				config.usage = WGPUTextureUsage_RenderAttachment;
				config.device = m_device;
				config.presentMode = WGPUPresentMode_Fifo;
				config.alphaMode = WGPUCompositeAlphaMode_Auto;

				wgpuSurfaceConfigure(m_surface, &config);
				LOG("Renderer Setup Complete!")
				m_initComplete = true;
			}
			else
			{
				LOG_ERR("Error requesting device: {}", message);
			}
		}

		static void OnQueueWorkDone(WGPUQueueWorkDoneStatus status, void* pUserData)
		{
			LOG("Work done ({})", status);
		}

		static void DeviceLostCallback(WGPUDeviceLostReason reason, char const* message, void* pUserData)
		{
			LOG_ERR("Device lost({}): {}", reason, message);
		}

		static void UncapturedErrorCallback(WGPUErrorType type, char const* message, void* pUserData)
		{
			LOG_ERR("Uncaptured device error({}): {}", type, message);
		}

		WGPUTextureView GetNextSurfaceTextureView()
		{
			WGPUSurfaceTexture surfaceTexture;
			wgpuSurfaceGetCurrentTexture(m_surface, &surfaceTexture);
			if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success)
			{
				return nullptr;
			}

			WGPUTextureViewDescriptor viewDescriptor;
			viewDescriptor.nextInChain = nullptr;
			viewDescriptor.label = "Surface texture view";
			viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
			viewDescriptor.dimension = WGPUTextureViewDimension_2D;
			viewDescriptor.baseMipLevel = 0;
			viewDescriptor.mipLevelCount = 1;
			viewDescriptor.baseArrayLayer = 0;
			viewDescriptor.arrayLayerCount = 1;
			viewDescriptor.aspect = WGPUTextureAspect_All;
			WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

			wgpuTextureRelease(surfaceTexture.texture);

			return targetView;
		}
	};

}
