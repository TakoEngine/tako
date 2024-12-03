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
			{
				float t = emscripten_run_script_int("performance.now()") / 100;
				UpdateUniform(&t, sizeof(float), offsetof(Uniforms, time));
			}
			m_targetView = GetNextSurfaceTextureView();
			ASSERT(m_targetView);

			WGPUCommandEncoderDescriptor encoderDesc = {};
			encoderDesc.nextInChain = nullptr;
			encoderDesc.label = "DefaultEncoder";
			m_encoder = wgpuDeviceCreateCommandEncoder(m_device, &encoderDesc);

			WGPURenderPassDescriptor renderPassDesc = {};
			renderPassDesc.nextInChain = nullptr;

			WGPURenderPassColorAttachment renderPassColorAttachment = {};
			renderPassColorAttachment.view = m_targetView;
			renderPassColorAttachment.resolveTarget = nullptr;
			renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
			renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
			renderPassColorAttachment.clearValue = WGPUColor{ 0.2, 0.2, 0.2, 1.0 };
			renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED; //Might cause problems with WGPU-native

			WGPURenderPassDepthStencilAttachment depthStencilAttachment = {};
			depthStencilAttachment.view = m_depthTextureView;
			depthStencilAttachment.depthClearValue = 1.0f;
			depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
			depthStencilAttachment.depthStoreOp = WGPUStoreOp_Store;
			depthStencilAttachment.depthReadOnly = false;

			depthStencilAttachment.stencilClearValue = 0;
			depthStencilAttachment.stencilLoadOp = WGPULoadOp_Undefined;
			depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Undefined;
			depthStencilAttachment.stencilReadOnly = true;

			renderPassDesc.colorAttachmentCount = 1;
			renderPassDesc.colorAttachments = &renderPassColorAttachment;
			renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
			renderPassDesc.timestampWrites = nullptr;


			m_renderPass = wgpuCommandEncoderBeginRenderPass(m_encoder, &renderPassDesc);

			//Render
			/*
			wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline);
			wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, m_pointBuffer, 0, wgpuBufferGetSize(m_pointBuffer));
			wgpuRenderPassEncoderSetIndexBuffer(renderPass, m_indexBuffer, WGPUIndexFormat_Uint16, 0, wgpuBufferGetSize(m_indexBuffer));
			wgpuRenderPassEncoderSetBindGroup(renderPass, 0, m_bindGroup, 0, nullptr);


			wgpuRenderPassEncoderDrawIndexed(renderPass, m_indexCount, 1, 0, 0, 0);
			*/

			//End

		}

		virtual void End() override
		{
			//wgpuTextureViewRelease(m_currentTargetView);
			//m_currentTargetView = nullptr;

			wgpuRenderPassEncoderEnd(m_renderPass);
			wgpuRenderPassEncoderRelease(m_renderPass);
			m_renderPass = nullptr;

			WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
			cmdBufferDescriptor.nextInChain = nullptr;
			cmdBufferDescriptor.label = "Command buffer";
			WGPUCommandBuffer command = wgpuCommandEncoderFinish(m_encoder, &cmdBufferDescriptor);
			wgpuCommandEncoderRelease(m_encoder);
			m_encoder = nullptr;

			wgpuQueueSubmit(m_queue, 1, &command);
			wgpuCommandBufferRelease(command);

			wgpuTextureViewRelease(m_targetView);
			m_targetView = nullptr;
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
			return m_window->GetWidth();
		}

		virtual U32 GetHeight() override
		{
			return m_window->GetHeight();
		}


		virtual void BindPipeline(const Pipeline* pipeline) override
		{
			WGPURenderPipeline wPipeline = reinterpret_cast<WGPURenderPipeline>(pipeline->value);
			wgpuRenderPassEncoderSetPipeline(m_renderPass, wPipeline);
		}

		virtual void BindVertexBuffer(const Buffer* buffer) override
		{
			WGPUBuffer wBuffer = reinterpret_cast<WGPUBuffer>(buffer->value);
			wgpuRenderPassEncoderSetVertexBuffer(m_renderPass, 0, wBuffer, 0, wgpuBufferGetSize(wBuffer));
		}

		virtual void BindIndexBuffer(const Buffer* buffer) override
		{
			WGPUBuffer wBuffer = reinterpret_cast<WGPUBuffer>(buffer->value);
			wgpuRenderPassEncoderSetIndexBuffer(m_renderPass, wBuffer, WGPUIndexFormat_Uint16, 0, wgpuBufferGetSize(wBuffer));
		}

		virtual void BindMaterial(const Material* material) override
		{

		}


		virtual void UpdateCamera(const CameraUniformData& cameraData) override
		{
			UpdateUniform(&cameraData.view, 2 * sizeof(Matrix4), offsetof(Uniforms, viewMatrix));
		}

		virtual void UpdateUniform(const void* uniformData, size_t uniformSize, size_t offset = 0) override
		{
			wgpuQueueWriteBuffer(m_queue, m_uniformBuffer, offset, uniformData, uniformSize);
		}


		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix) override
		{
			UpdateUniform(&renderMatrix, sizeof(Matrix4), offsetof(Uniforms, modelMatrix));
			wgpuRenderPassEncoderSetBindGroup(m_renderPass, 0, m_bindGroup, 0, nullptr);
			wgpuRenderPassEncoderDrawIndexed(m_renderPass, indexCount, 1, 0, 0, 0);
		}

		virtual void DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix) override
		{

		}


		virtual Pipeline CreatePipeline(const PipelineDescriptor& pipelineDescriptor) override
		{
			const char* shaderSource = R"(
				struct Uniforms {
					viewMatrix: mat4x4f,
					projectionMatrix: mat4x4f,
					modelMatrix: mat4x4f,
					color: vec4f,
					time: f32,
				};

				@group(0) @binding(0) var<uniform> uniforms: Uniforms;
				@group(0) @binding(1) var gradientTexture: texture_2d<f32>;
				@group(0) @binding(2) var textureSampler: sampler;

				struct VertexInput {
					@location(0) position: vec3f,
					@location(1) normal: vec3f,
					@location(2) color: vec3f,
					@location(3) uv: vec2f,
				};

				struct VertexOutput {
					@builtin(position) position: vec4f,
					@location(0) color: vec3f,
					@location(1) normal: vec3f,
					@location(2) uv: vec2f,
				}

				@vertex
				fn vs_main(in: VertexInput) -> VertexOutput {
					var out: VertexOutput;
					out.position = uniforms.projectionMatrix * uniforms.viewMatrix * uniforms.modelMatrix * vec4f(in.position, 1.0);
					out.color = in.color;
					out.normal = (uniforms.modelMatrix * vec4f(in.normal, 0.0)).xyz;
					out.uv = in.uv;
					return out;
				}

				@fragment
				fn fs_main(in: VertexOutput) -> @location(0) vec4f {
					let color = textureSample(gradientTexture, textureSampler, in.uv).rgb;
					//let normal = normalize(in.normal);
					//let lightDirection = vec3f(0.5, -0.9, 0.1);
					//let shading = dot(lightDirection, normal);
					//let color = in.color * shading;

					return vec4f(color, 1.0);
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
			std::vector<WGPUVertexAttribute> vertexAttribs(pipelineDescriptor.vertexAttributeSize);
			size_t stride = 0;
			for (int i = 0; i < pipelineDescriptor.vertexAttributeSize; i++)
			{
				WGPUVertexFormat format;
				size_t attribSize;
				switch (pipelineDescriptor.vertexAttributes[i])
				{
					case PipelineVectorAttribute::Vec2:
						format = WGPUVertexFormat_Float32x2;
						attribSize = 2;
						break;
					case PipelineVectorAttribute::Vec3:
						format = WGPUVertexFormat_Float32x3;
						attribSize = 3;
						break;
				}
				vertexAttribs[i].shaderLocation = i;
				vertexAttribs[i].format = format;
				vertexAttribs[i].offset = stride;
				stride += attribSize * sizeof(float);
			}

			vertexBufferLayout.attributeCount = vertexAttribs.size();
			vertexBufferLayout.attributes = vertexAttribs.data();
			vertexBufferLayout.arrayStride = stride;
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

			WGPUDepthStencilState depthStencilState;
			SetDefault(depthStencilState);
			depthStencilState.depthCompare = WGPUCompareFunction_Less;
			depthStencilState.depthWriteEnabled = true;
			depthStencilState.format = m_depthTextureFormat;
			depthStencilState.stencilReadMask = 0;
			depthStencilState.stencilWriteMask = 0;

			pipelineDesc.depthStencil = &depthStencilState;

			pipelineDesc.multisample.count = 1;
			pipelineDesc.multisample.mask = ~0u;
			pipelineDesc.multisample.alphaToCoverageEnabled = false;

			std::array<WGPUBindGroupLayoutEntry, 3> bindingLayoutEntries;

			WGPUBindGroupLayoutEntry& bindingLayout = bindingLayoutEntries[0];
			SetDefault(bindingLayout);
			bindingLayout.binding = 0;
			bindingLayout.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
			bindingLayout.buffer.type = WGPUBufferBindingType_Uniform;
			bindingLayout.buffer.minBindingSize = sizeof(Uniforms);

			WGPUBindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries[1];
			SetDefault(textureBindingLayout);
			textureBindingLayout.binding = 1;
			textureBindingLayout.visibility = WGPUShaderStage_Fragment;
			textureBindingLayout.texture.sampleType = WGPUTextureSampleType_Float;
			textureBindingLayout.texture.viewDimension = WGPUTextureViewDimension_2D;

			WGPUBindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries[2];
			SetDefault(samplerBindingLayout);
			samplerBindingLayout.binding = 2;
			samplerBindingLayout.visibility = WGPUShaderStage_Fragment;
			samplerBindingLayout.sampler.type = WGPUSamplerBindingType_Filtering;

			WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc{};
			bindGroupLayoutDesc.nextInChain = nullptr;
			bindGroupLayoutDesc.entryCount = bindingLayoutEntries.size();
			bindGroupLayoutDesc.entries = bindingLayoutEntries.data();
			m_bindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_device, &bindGroupLayoutDesc);

			WGPUPipelineLayoutDescriptor layoutDesc{};
			layoutDesc.nextInChain = nullptr;
			layoutDesc.bindGroupLayoutCount = 1;
			layoutDesc.bindGroupLayouts = &m_bindGroupLayout;
			m_layout = wgpuDeviceCreatePipelineLayout(m_device, &layoutDesc);

			pipelineDesc.layout = m_layout;

			WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(m_device, &pipelineDesc);
			m_pipeline = pipeline;

			wgpuShaderModuleRelease(shaderModule);

			// Create uniforms
			Uniforms uniforms;
			uniforms.projectionMatrix = Matrix4::perspective(45, GetWidth() / (float) GetHeight(), 1, 1000);
			uniforms.viewMatrix = Matrix4::cameraViewMatrix(Vector3(0, 0, 0), {});
			uniforms.modelMatrix = Matrix4::identity;
			uniforms.time = 1.0f;
			uniforms.color = { 1.0f, 1.0f, 0.4f, 1.0f };
			m_uniformBuffer = CreateWGPUBuffer(WGPUBufferUsage_Uniform, &uniforms, sizeof(Uniforms));

			std::vector<WGPUBindGroupEntry> bindings(3);

			bindings[0].nextInChain = nullptr;
			bindings[0].binding = 0;
			bindings[0].buffer = m_uniformBuffer;
			bindings[0].offset = 0;
			bindings[0].size = sizeof(Uniforms);

			WGPUTextureView textureView = reinterpret_cast<WGPUTextureView>(CreateTexture(Bitmap::FromFile("/CrossGolf.png")).ptr);

			bindings[1].nextInChain = nullptr;
			bindings[1].binding = 1;
			bindings[1].textureView = textureView;

			WGPUSamplerDescriptor samplerDesc;
			samplerDesc.nextInChain = nullptr;
			samplerDesc.addressModeU = WGPUAddressMode_ClampToEdge;
			samplerDesc.addressModeV = WGPUAddressMode_ClampToEdge;
			samplerDesc.addressModeW = WGPUAddressMode_ClampToEdge;
			samplerDesc.magFilter = WGPUFilterMode_Linear;
			samplerDesc.minFilter = WGPUFilterMode_Linear;
			samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
			samplerDesc.lodMinClamp = 0.0f;
			samplerDesc.lodMaxClamp = 1.0f;
			samplerDesc.compare = WGPUCompareFunction_Undefined;
			samplerDesc.maxAnisotropy = 1;
			WGPUSampler sampler = wgpuDeviceCreateSampler(m_device, &samplerDesc);

			bindings[2].binding = 2;
			bindings[2].sampler = sampler;

			WGPUBindGroupDescriptor bindGroupDesc{};
			bindGroupDesc.nextInChain = nullptr;
			bindGroupDesc.layout = m_bindGroupLayout;

			bindGroupDesc.entryCount = bindings.size();
			bindGroupDesc.entries = bindings.data();
			m_bindGroup = wgpuDeviceCreateBindGroup(m_device, &bindGroupDesc);

			CreateDepthTexture();

			//TODO: manage pipeline lifetime
			return {reinterpret_cast<U64>(pipeline)};
		}

		virtual Material CreateMaterial(const Texture* texture) override
		{
			return {};
		}

		virtual Texture CreateTexture(const Bitmap& bitmap) override
		{
			WGPUTextureDescriptor textureDesc;
			textureDesc.nextInChain = nullptr;
			textureDesc.dimension = WGPUTextureDimension_2D;
			textureDesc.size = { (U32) bitmap.Width(), (U32) bitmap.Height(), 1 };
			textureDesc.mipLevelCount = 1;
			textureDesc.sampleCount = 1;
			textureDesc.format = m_textureFormat;
			textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
			textureDesc.viewFormatCount = 0;
			textureDesc.viewFormats = nullptr;

			WGPUTexture texture = wgpuDeviceCreateTexture(m_device, &textureDesc);

			WGPUTextureViewDescriptor textureViewDesc;
			textureViewDesc.nextInChain = nullptr;
			textureViewDesc.aspect = WGPUTextureAspect_All;
			textureViewDesc.baseArrayLayer = 0;
			textureViewDesc.arrayLayerCount = 1;
			textureViewDesc.baseMipLevel = 0;
			textureViewDesc.mipLevelCount = 1;
			textureViewDesc.dimension = WGPUTextureViewDimension_2D;
			textureViewDesc.format = m_textureFormat;
			WGPUTextureView textureView = wgpuTextureCreateView(texture, &textureViewDesc);
			ASSERT(textureView != nullptr);


			WGPUImageCopyTexture destination;
			destination.nextInChain = nullptr;
			destination.texture = texture;
			destination.mipLevel = 0;
			destination.origin = { 0, 0, 0 };
			destination.aspect = WGPUTextureAspect_All;

			WGPUTextureDataLayout source;
			source.nextInChain = nullptr;
			source.offset = 0;
			source.bytesPerRow = 4 * textureDesc.size.width;
			source.rowsPerImage = textureDesc.size.height;

			size_t imageSize = sizeof(Color) * (U32) bitmap.Width() *  (U32) bitmap.Height();
			wgpuQueueWriteTexture(m_queue, &destination, bitmap.GetData(), imageSize, &source, &textureDesc.size);

			Texture tex;
			tex.handle.value = reinterpret_cast<U64>(texture);
			tex.ptr = textureView;
			return tex;
		}

		virtual Buffer CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize) override
		{
			WGPUBufferUsage usage;
			switch (bufferType)
			{
				case BufferType::Vertex:
					usage = WGPUBufferUsage_Vertex;
					break;
				case BufferType::Index:
					usage = WGPUBufferUsage_Index;
					break;
			}

			WGPUBuffer buffer = CreateWGPUBuffer(usage, bufferData, bufferSize);

			return {reinterpret_cast<U64>(buffer)};
		}

	private:


		//TEMP
		struct Uniforms
		{
			Matrix4 viewMatrix;
			Matrix4 projectionMatrix;
			Matrix4 modelMatrix;
			std::array<float, 4> color;
			float time;
			float _pad[3];
		};
		static_assert(sizeof(Uniforms) % 16 == 0);
		WGPURenderPipeline m_pipeline;
		U32 m_indexCount;
		WGPUBuffer m_pointBuffer;
		WGPUBuffer m_indexBuffer;
		WGPUBuffer m_uniformBuffer;
		WGPUPipelineLayout m_layout;
    	WGPUBindGroupLayout m_bindGroupLayout;
		WGPUBindGroup m_bindGroup;

		WGPUTextureFormat m_depthTextureFormat = WGPUTextureFormat_Depth24Plus;
		WGPUTexture m_depthTexture;
		WGPUTextureView m_depthTextureView;
		WGPUTextureFormat m_textureFormat = WGPUTextureFormat_RGBA8Unorm;

		WGPURenderPassEncoder m_renderPass;
		WGPUTextureView m_targetView;
		WGPUCommandEncoder m_encoder;

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

		WGPUBuffer CreateWGPUBuffer(WGPUBufferUsage bufferType, const void* bufferData, size_t dataSize)
		{
			return CreateWGPUBuffer(bufferType, bufferData, dataSize, dataSize);
		}

		WGPUBuffer CreateWGPUBuffer(WGPUBufferUsage bufferType, const void* bufferData, size_t dataSize, size_t size)
		{
			WGPUBufferDescriptor bufferDesc{};
			bufferDesc.nextInChain = nullptr;
			bufferDesc.size = size;
			bufferDesc.usage = WGPUBufferUsage_CopyDst | bufferType;
			bufferDesc.mappedAtCreation = false;
			WGPUBuffer buffer = wgpuDeviceCreateBuffer(m_device, &bufferDesc);

			wgpuQueueWriteBuffer(m_queue, buffer, 0, bufferData, dataSize);

			return buffer;
		}

		void CreateDepthTexture()
		{
			WGPUTextureDescriptor depthTextureDesc;
			depthTextureDesc.nextInChain = nullptr;
			depthTextureDesc.dimension = WGPUTextureDimension_2D;
			depthTextureDesc.format = m_depthTextureFormat;
			depthTextureDesc.mipLevelCount = 1;
			depthTextureDesc.sampleCount = 1;
			depthTextureDesc.size = { (U32)m_window->GetWidth(), (U32) m_window->GetHeight(), 1};
			depthTextureDesc.usage = WGPUTextureUsage_RenderAttachment;
			depthTextureDesc.viewFormatCount = 1;
			depthTextureDesc.viewFormats = &m_depthTextureFormat;
			m_depthTexture = wgpuDeviceCreateTexture(m_device, &depthTextureDesc);

			WGPUTextureViewDescriptor depthTextureViewDesc;
			depthTextureViewDesc.nextInChain = nullptr;
			depthTextureViewDesc.aspect = WGPUTextureAspect_DepthOnly;
			depthTextureViewDesc.baseArrayLayer = 0;
			depthTextureViewDesc.arrayLayerCount = 1;
			depthTextureViewDesc.baseMipLevel = 0;
			depthTextureViewDesc.mipLevelCount = 1;
			depthTextureViewDesc.dimension = WGPUTextureViewDimension_2D;
			depthTextureViewDesc.format = m_depthTextureFormat;
			m_depthTextureView = wgpuTextureCreateView(m_depthTexture, &depthTextureViewDesc);
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

		void SetDefault(WGPUBindGroupLayoutEntry &bindingLayout) const
		{
			bindingLayout.buffer.nextInChain = nullptr;
			bindingLayout.buffer.type = WGPUBufferBindingType_Undefined;
			bindingLayout.buffer.hasDynamicOffset = false;

			bindingLayout.sampler.nextInChain = nullptr;
			bindingLayout.sampler.type = WGPUSamplerBindingType_Undefined;

			bindingLayout.storageTexture.nextInChain = nullptr;
			bindingLayout.storageTexture.access = WGPUStorageTextureAccess_Undefined;
			bindingLayout.storageTexture.format = WGPUTextureFormat_Undefined;
			bindingLayout.storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

			bindingLayout.texture.nextInChain = nullptr;
			bindingLayout.texture.multisampled = false;
			bindingLayout.texture.sampleType = WGPUTextureSampleType_Undefined;
			bindingLayout.texture.viewDimension = WGPUTextureViewDimension_Undefined;
		}

		void SetDefault(WGPUStencilFaceState &stencilFaceState) const
		{
			stencilFaceState.compare = WGPUCompareFunction_Always;
			stencilFaceState.failOp = WGPUStencilOperation_Keep;
			stencilFaceState.depthFailOp = WGPUStencilOperation_Keep;
			stencilFaceState.passOp = WGPUStencilOperation_Keep;
		}

		void SetDefault(WGPUDepthStencilState &depthStencilState) const
		{
			depthStencilState.format = WGPUTextureFormat_Undefined;
			depthStencilState.depthWriteEnabled = false;
			depthStencilState.depthCompare = WGPUCompareFunction_Always;
			depthStencilState.stencilReadMask = 0xFFFFFFFF;
			depthStencilState.stencilWriteMask = 0xFFFFFFFF;
			depthStencilState.depthBias = 0;
			depthStencilState.depthBiasSlopeScale = 0;
			depthStencilState.depthBiasClamp = 0;
			SetDefault(depthStencilState.stencilFront);
			SetDefault(depthStencilState.stencilBack);
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
			requiredLimits.limits.maxBufferSize = 10000 * 11 * sizeof(float);
			requiredLimits.limits.maxVertexBufferArrayStride = 11 * sizeof(float);
			requiredLimits.limits.maxInterStageShaderComponents = 8;
			requiredLimits.limits.maxBindGroups = 1;
			requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
			requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
			requiredLimits.limits.maxTextureDimension1D = 2160;
			requiredLimits.limits.maxTextureDimension2D = 3840;
			requiredLimits.limits.maxTextureArrayLayers = 1;
			requiredLimits.limits.maxSampledTexturesPerShaderStage = 1;
			requiredLimits.limits.maxSamplersPerShaderStage = 1;

			requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
			requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;

			return requiredLimits;
		}
	};

}
