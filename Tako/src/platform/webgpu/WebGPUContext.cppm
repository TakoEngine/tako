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

			//Render
			wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline);
			wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, m_vertexBuffer, 0, wgpuBufferGetSize(m_vertexBuffer));
			wgpuRenderPassEncoderDraw(renderPass, m_vertexCount, 1, 0, 0);

			//End
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
			const char* shaderSource = R"(

				struct VertexInput {
					@location(0) position: vec2f,
					@location(1) color: vec3f,
				};

				struct VertexOutput {
					@builtin(position) position: vec4f,
					@location(0) color: vec3f,
				}

				@vertex
				fn vs_main(in: VertexInput) -> VertexOutput {
					var out: VertexOutput;
					out.position = vec4f(in.position, 0.0, 1.0);
					out.color = in.color;
					return out;
				}

				@fragment
				fn fs_main(in: VertexOutput) -> @location(0) vec4f {
					return vec4f(in.color, 1.0);
				}
			)";

			WGPUShaderModuleDescriptor shaderDesc{};

			WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
			shaderCodeDesc.chain.next = nullptr;
			shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
			shaderCodeDesc.code = shaderSource;

			shaderDesc.nextInChain = &shaderCodeDesc.chain;
			shaderCodeDesc.code = shaderSource;
			WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(m_device, &shaderDesc);

			WGPURenderPipelineDescriptor pipelineDesc{};
			pipelineDesc.nextInChain = nullptr;

			WGPUVertexBufferLayout vertexBufferLayout{};
			std::array<WGPUVertexAttribute, 2> vertexAttribs;

			// position
			vertexAttribs[0].shaderLocation = 0;
			vertexAttribs[0].format = WGPUVertexFormat_Float32x2;
			vertexAttribs[0].offset = 0;

			// color
			vertexAttribs[1].shaderLocation = 1;
			vertexAttribs[1].format = WGPUVertexFormat_Float32x3;
			vertexAttribs[1].offset = 2 * sizeof(float);

			vertexBufferLayout.attributeCount = vertexAttribs.size();
			vertexBufferLayout.attributes = vertexAttribs.data();
			vertexBufferLayout.arrayStride = 5 * sizeof(float);
			vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;

			pipelineDesc.vertex.bufferCount = 1;
			pipelineDesc.vertex.buffers = &vertexBufferLayout;

			pipelineDesc.vertex.module = shaderModule;
			pipelineDesc.vertex.entryPoint = "vs_main";
			pipelineDesc.vertex.constantCount = 0;
			pipelineDesc.vertex.constants = nullptr;

			pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
			pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
			pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
			pipelineDesc.primitive.cullMode = WGPUCullMode_None;

			WGPUFragmentState fragmentState{};
			fragmentState.module = shaderModule;
			fragmentState.entryPoint = "fs_main";
			fragmentState.constantCount = 0;
			fragmentState.constants = nullptr;

			//Blending
			WGPUBlendState blendState{};
			blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
			blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
			blendState.color.operation = WGPUBlendOperation_Add;
			blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
			blendState.alpha.dstFactor = WGPUBlendFactor_One;
			blendState.alpha.operation = WGPUBlendOperation_Add;

			WGPUColorTargetState colorTarget{};
			colorTarget.format = m_surfaceFormat;
			colorTarget.blend = &blendState;
			colorTarget.writeMask = WGPUColorWriteMask_All;

			fragmentState.targetCount = 1;
			fragmentState.targets = &colorTarget;
			pipelineDesc.fragment = &fragmentState;

			pipelineDesc.depthStencil = nullptr;
			pipelineDesc.multisample.count = 1;
			pipelineDesc.multisample.mask = ~0u;
			pipelineDesc.multisample.alphaToCoverageEnabled = false;
			pipelineDesc.layout = nullptr;

			WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(m_device, &pipelineDesc);
			m_pipeline = pipeline;

			wgpuShaderModuleRelease(shaderModule);
			//TODO: manage pipeline lifetime
			return {reinterpret_cast<U64>(pipeline)};
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
			std::vector<float> vertexData =
			{
				// x0,  y0,  r0,  g0,  b0
				-0.5, -0.5, 1.0, 0.0, 0.0,

				// x1,  y1,  r1,  g1,  b1
				+0.5, -0.5, 0.0, 1.0, 0.0,

				+0.0,   +0.5, 0.0, 0.0, 1.0,
				-0.55f, -0.5, 1.0, 1.0, 0.0,
				-0.05f, +0.5, 1.0, 0.0, 1.0,
				-0.55f, +0.5, 0.0, 1.0, 1.0
			};

			m_vertexCount = static_cast<uint32_t>(vertexData.size() / 5);

			WGPUBufferDescriptor bufferDesc{};
			bufferDesc.nextInChain = nullptr;
			bufferDesc.size = vertexData.size() * sizeof(float);
			bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
			bufferDesc.mappedAtCreation = false;
			m_vertexBuffer = wgpuDeviceCreateBuffer(m_device, &bufferDesc);

			wgpuQueueWriteBuffer(m_queue, m_vertexBuffer, 0, vertexData.data(), bufferDesc.size);

			return {};
		}
	private:
		WGPUTextureView m_currentTargetView;

		//TEMP
		WGPURenderPipeline m_pipeline;
		U32 m_vertexCount;
		WGPUBuffer m_vertexBuffer;

		bool m_initComplete = false; //TODO: tie into "await" system
		Window* m_window;
		WGPUQueue m_queue;
		WGPUDevice m_device;
		WGPUAdapter m_adapter;
		WGPUTextureFormat m_surfaceFormat = WGPUTextureFormat_Undefined;
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
				WGPURequiredLimits requiredLimits = GetRequiredLimits(adapter);
				deviceDesc.requiredLimits = &requiredLimits;
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

				m_surfaceFormat = wgpuSurfaceGetPreferredFormat(m_surface, m_adapter);
				config.format = m_surfaceFormat;
				config.viewFormatCount = 0;
				config.viewFormats = nullptr;
				config.usage = WGPUTextureUsage_RenderAttachment;
				config.device = m_device;
				config.presentMode = WGPUPresentMode_Fifo;
				config.alphaMode = WGPUCompositeAlphaMode_Auto;

				wgpuSurfaceConfigure(m_surface, &config);
				LOG("Renderer Setup Complete!")
				m_initComplete = true;

				WGPUSupportedLimits supportedLimits{};
				supportedLimits.nextInChain = nullptr;

				wgpuAdapterGetLimits(m_adapter, &supportedLimits);
				LOG("adapter.maxVertexAttributes: {}", supportedLimits.limits.maxVertexAttributes);

				wgpuDeviceGetLimits(m_device, &supportedLimits);
				LOG("device.maxVertexAttributes: {}", supportedLimits.limits.maxVertexAttributes);

				CreateBuffer(tako::BufferType::Vertex, nullptr, 0);
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

		void SetDefault(WGPULimits& limits) const
		{
			limits.maxTextureDimension1D = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxTextureDimension2D = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxTextureDimension3D = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxTextureArrayLayers = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxBindGroups = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxBindGroupsPlusVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxBindingsPerBindGroup = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxDynamicUniformBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxDynamicStorageBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxSampledTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxSamplersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxStorageBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxStorageTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxUniformBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxUniformBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
			limits.maxStorageBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
			limits.minUniformBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
			limits.minStorageBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxBufferSize = WGPU_LIMIT_U64_UNDEFINED;
			limits.maxVertexAttributes = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxVertexBufferArrayStride = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxInterStageShaderComponents = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxInterStageShaderVariables = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxColorAttachments = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxColorAttachmentBytesPerSample = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxComputeWorkgroupStorageSize = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxComputeInvocationsPerWorkgroup = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxComputeWorkgroupSizeX = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxComputeWorkgroupSizeY = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxComputeWorkgroupSizeZ = WGPU_LIMIT_U32_UNDEFINED;
			limits.maxComputeWorkgroupsPerDimension = WGPU_LIMIT_U32_UNDEFINED;
		}

		WGPURequiredLimits GetRequiredLimits(WGPUAdapter adapter) const
		{
			WGPUSupportedLimits supportedLimits;
			supportedLimits.nextInChain = nullptr;
			wgpuAdapterGetLimits(adapter, &supportedLimits);

			WGPURequiredLimits requiredLimits{};
			SetDefault(requiredLimits.limits);

			requiredLimits.limits.maxVertexAttributes = 2;
			requiredLimits.limits.maxVertexBuffers = 1;
			requiredLimits.limits.maxBufferSize = 6 * 5 * sizeof(float);
			requiredLimits.limits.maxVertexBufferArrayStride = 5 * sizeof(float);
			requiredLimits.limits.maxInterStageShaderComponents = 3;

			requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
			requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;

			return requiredLimits;
		}
	};

}
