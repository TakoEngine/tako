module;
#include "Utility.hpp"
#include <IGraphicsContext.hpp>
#include <webgpu/webgpu_cpp.h>
#if defined(TAKO_EMSCRIPTEN)
#include <emscripten.h>
#elif defined(TAKO_GLFW)
#include <webgpu/webgpu_glfw.h>
#endif
#include <bit>
export module Tako.WebGPU;

import Tako.JobSystem;
import Tako.StringView;
import Tako.HandleVec;
import Tako.SmallVec;

class WStringView : public tako::CStringView
{
public:
	#ifndef TAKO_EMSCRIPTEN
	operator WGPUStringView()
	{
		return {m_str, m_len};
	}

	operator const WGPUStringView() const
	{
		return {m_str, m_len};
	}
	#endif
};

#ifndef TAKO_EMSCRIPTEN
using WLabel = WGPUStringView;

template <>
class fmt::formatter<WGPUStringView> {
public:
	constexpr auto parse (format_parse_context& ctx) { return ctx.begin(); }
	template <typename Context>
	constexpr auto format (WGPUStringView const& str, Context& ctx) const {
		std::string_view s(str.data, str.length);
		return format_to(ctx.out(), "{}", s);
	}
};
#else
using WLabel = const char*;
#endif

#ifdef TAKO_EMSCRIPTEN
#define WGPUBufferBindingType_BindingNotUsed WGPUBufferBindingType_Undefined
#endif

#ifdef TAKO_EMSCRIPTEN
#define WGPUSamplerBindingType_BindingNotUsed WGPUSamplerBindingType_Undefined
#endif

#ifdef TAKO_EMSCRIPTEN
#define WGPUStorageTextureAccess_BindingNotUsed WGPUStorageTextureAccess_Undefined
#endif

#ifdef TAKO_EMSCRIPTEN
#define WGPUTextureSampleType_BindingNotUsed WGPUTextureSampleType_Undefined
#endif

class WBool
{
public:
	WBool(bool b) : m_bool(b) {}

	operator bool()
	{
		return m_bool;
	}

#ifndef TAKO_EMSCRIPTEN
	operator WGPUOptionalBool()
	{
		if (m_bool)
		{
			return WGPUOptionalBool_True;
		}
		else
		{
			return WGPUOptionalBool_False;
		}
	}
#endif
private:
	bool m_bool;
};


namespace tako
{
	struct InstanceBuffer
	{
		wgpu::Buffer buffer;
		wgpu::BindGroup group;
		size_t currentIndex = 0;
		size_t size;
	};

	struct CameraBuffer
	{
		wgpu::Buffer buffer;
		wgpu::BindGroup group;
	};

	struct FrameState
	{
		size_t usedCameraBuffers = 0;
	};

	struct TextureEntry
	{
		wgpu::Texture texture;
		wgpu::TextureView view;
	};

	struct PipelineEntry
	{
		wgpu::RenderPipeline pipeline;
		std::vector<ShaderBindingLayout> layouts;
		U16 modelBinding;
	};

	wgpu::TextureViewDimension ConvertToWGPU(TextureType type);

	export class WebGPUContext final : public IGraphicsContext
	{
	public:
		WebGPUContext(Window* window)
		{
			auto frameBufferSize = window->GetFramebufferSize();
			m_width = frameBufferSize.x;
			m_height = frameBufferSize.y;
			wgpu::InstanceDescriptor desc;

			wgpu::Instance instance;


#ifdef EMSCRIPTEN
			instance = wgpu::CreateInstance(nullptr);
#else
			wgpu::DawnTogglesDescriptor toggles;
			toggles.sType = wgpu::SType::DawnTogglesDescriptor;
			toggles.disabledToggleCount = 0;
			toggles.enabledToggleCount = 1;
			const char* toggleName = "enable_immediate_error_handling";
			toggles.enabledToggles = &toggleName;

			desc.nextInChain = &toggles;
#ifndef TAKO_WIN32
			instance = wgpu::CreateInstance(&desc);
#else
			// Workaround required to compile under windows
			{
				auto result = wgpuCreateInstance(reinterpret_cast<WGPUInstanceDescriptor const*>(&desc));
				instance = wgpu::Instance::Acquire(result);
			}
#endif
#endif
			ASSERT(instance);
			m_instance = instance;

			// Surface
#if defined(TAKO_EMSCRIPTEN)
			wgpu::SurfaceDescriptorFromCanvasHTMLSelector fromCanvasHTMLSelector;
			fromCanvasHTMLSelector.sType = wgpu::SType::SurfaceDescriptorFromCanvasHTMLSelector;
			fromCanvasHTMLSelector.selector = window->GetHandle();

			wgpu::SurfaceDescriptor surfaceDescriptor;
			surfaceDescriptor.nextInChain = &fromCanvasHTMLSelector;
			surfaceDescriptor.label = nullptr;

			m_surface = m_instance.CreateSurface(&surfaceDescriptor);
#elif defined(TAKO_GLFW)
			m_surface = wgpu::glfw::CreateSurfaceForWindow(m_instance, window->GetHandle());
#endif
			ASSERT(m_surface);

			wgpu::RequestAdapterOptions adapterOpts;
			adapterOpts.compatibleSurface = m_surface;

			m_instance.RequestAdapter(
				&adapterOpts,
				[](WGPURequestAdapterStatus status, WGPUAdapter adapter, WLabel message, void* pUserData)
				{
					reinterpret_cast<WebGPUContext*>(pUserData)->RequestAdapterCallback(status, adapter, message);
				},
				this
			);

			while (!m_initComplete)
			{
#ifdef TAKO_EMSCRIPTEN
				emscripten_sleep(100);
#endif
			}
		}

		virtual ~WebGPUContext() override
		{
			m_queue = nullptr;
			m_device = nullptr;
			m_surface.Unconfigure();
			m_surface = nullptr;
			m_instance = nullptr;
		}

		virtual GraphicsAPI GetAPI() override
		{
			return GraphicsAPI::WebGPU;
		}


		virtual void Begin() override
		{
			{
				//float t = emscripten_run_script_int("performance.now()") / 100;
				//UpdateUniform(&t, sizeof(float), offsetof(Uniforms, time));
			}
			m_targetView = GetNextSurfaceTextureView();
			ASSERT(m_targetView);

			wgpu::CommandEncoderDescriptor encoderDesc;
			encoderDesc.label = "DefaultEncoder";
			m_encoder = m_device.CreateCommandEncoder(&encoderDesc).MoveToCHandle();

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
			m_instanceBuffer.currentIndex = 0;
			m_currentFrame = {};

			//wgpuRenderPassEncoderSetViewport(m_renderPass, 0, 0, m_width, m_height, 0, 1);

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
			cmdBufferDescriptor.label = WStringView("Command buffer");
			wgpu::CommandBuffer command = wgpu::CommandBuffer::Acquire(wgpuCommandEncoderFinish(m_encoder, &cmdBufferDescriptor));
			wgpuCommandEncoderRelease(m_encoder);
			m_encoder = nullptr;

			m_queue.Submit(1, &command);

			wgpuTextureViewRelease(m_targetView);
			m_targetView = nullptr;
		}

		virtual void Present() override
		{
#ifndef EMSCRIPTEN
			m_surface.Present();
#endif
		}

		virtual void Resize(int width, int height) override
		{
			m_width = width;
			m_height = height;

			TerminateDepthTexture();

			ConfigureSurface();
			CreateDepthTexture();
		}

		virtual void HandleEvent(Event &evt) override
		{

		}


		virtual U32 GetWidth() override
		{
			return m_width;
		}

		virtual U32 GetHeight() override
		{
			return m_height;
		}


		virtual void BindPipeline(const Pipeline* pipeline) override
		{
			auto& entry = m_pipelines[*pipeline];
			wgpuRenderPassEncoderSetPipeline(m_renderPass, entry.pipeline.Get());
			m_currentModelBinding = entry.modelBinding;
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

		void Bind(const ShaderBinding binding, const U32 slot) override
		{
			auto& bindGroup = m_shaderBindings[binding];
			wgpuRenderPassEncoderSetBindGroup(m_renderPass, slot, bindGroup.Get(), 0, nullptr);
		}

		virtual void UpdateBuffer(Buffer buffer, const void* data, size_t writeSize, size_t offset = 0) override
		{
			auto buf = reinterpret_cast<WGPUBuffer>(buffer.value);
			m_queue.WriteBuffer(buf, offset, data, writeSize);
		}

		virtual void Draw(U32 vertexCount) override
		{
			wgpuRenderPassEncoderDraw(m_renderPass, vertexCount, 1, 0, 0);
		}

		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix) override
		{
			DrawIndexed(indexCount, 1, &renderMatrix);
		}

		virtual void DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix) override
		{
			if (m_instanceBuffer.currentIndex + matrixCount > m_instanceBuffer.size)
			{
				CreateInstanceBuffer(std::max<size_t>(m_instanceBuffer.size * 2, matrixCount));
			}
			uint64_t bufferOffset = m_instanceBuffer.currentIndex * sizeof(Matrix4);
			m_queue.WriteBuffer(m_instanceBuffer.buffer, bufferOffset, renderMatrix, matrixCount * sizeof(Matrix4));
			wgpuRenderPassEncoderSetBindGroup(m_renderPass, m_currentModelBinding, m_instanceBuffer.group.Get(), 0, nullptr);
			wgpuRenderPassEncoderDrawIndexed(m_renderPass, indexCount, matrixCount, 0, 0, m_instanceBuffer.currentIndex);
			m_instanceBuffer.currentIndex += matrixCount;
		}


		virtual Pipeline CreatePipeline(const PipelineDescriptor& pipelineDescriptor) override
		{
			WGPUShaderModuleDescriptor shaderDesc{};

			WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
			shaderCodeDesc.chain.next = nullptr;
#ifdef TAKO_EMSCRIPTEN
			shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
#else
			shaderCodeDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
#endif
			shaderCodeDesc.code = WStringView(pipelineDescriptor.shaderCode);

			shaderDesc.nextInChain = &shaderCodeDesc.chain;
			shaderCodeDesc.code = WStringView(pipelineDescriptor.shaderCode);
			WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(m_device.Get(), &shaderDesc);

			WGPURenderPipelineDescriptor pipelineDesc{};
			pipelineDesc.nextInChain = nullptr;

			WGPUVertexBufferLayout vertexBufferLayout{};
			std::vector<WGPUVertexAttribute> vertexAttribs(pipelineDescriptor.vertexAttributeSize);
			if (pipelineDescriptor.vertexAttributeSize > 0)
			{
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
						case PipelineVectorAttribute::Vec4:
							format = WGPUVertexFormat_Float32x4;
							attribSize = 4;
							break;
						case PipelineVectorAttribute::RGBA:
							format = WGPUVertexFormat_Unorm8x4;
							attribSize = 1;
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
			}
			else
			{
				pipelineDesc.vertex.bufferCount = 0;
				pipelineDesc.vertex.buffers = nullptr;
			}

			pipelineDesc.vertex.module = shaderModule;
			pipelineDesc.vertex.entryPoint = WStringView(pipelineDescriptor.vertEntry);
			pipelineDesc.vertex.constantCount = 0;
			pipelineDesc.vertex.constants = nullptr;

			pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
			pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
			pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
			pipelineDesc.primitive.cullMode = WGPUCullMode_None;

			WGPUFragmentState fragmentState{};
			fragmentState.module = shaderModule;
			fragmentState.entryPoint = WStringView(pipelineDescriptor.fragEntry);
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
			colorTarget.format = (WGPUTextureFormat) m_surfaceFormat;
			colorTarget.blend = &blendState;
			colorTarget.writeMask = WGPUColorWriteMask_All;

			fragmentState.targetCount = 1;
			fragmentState.targets = &colorTarget;
			pipelineDesc.fragment = &fragmentState;

			WGPUDepthStencilState depthStencilState;
			SetDefault(depthStencilState);
			depthStencilState.depthCompare = WGPUCompareFunction_LessEqual;
			depthStencilState.depthWriteEnabled = WBool(true);
			depthStencilState.format = m_depthTextureFormat;
			depthStencilState.stencilReadMask = 0;
			depthStencilState.stencilWriteMask = 0;

			pipelineDesc.depthStencil = &depthStencilState;

			pipelineDesc.multisample.count = 1;
			pipelineDesc.multisample.mask = ~0u;
			pipelineDesc.multisample.alphaToCoverageEnabled = false;

			PipelineEntry pipelineEntry;
			std::vector<wgpu::BindGroupLayout> layouts; //TODO: Use SmallVec
			for (auto& binding : pipelineDescriptor.shaderBindings)
			{
				wgpu::BindGroupLayout layout = CreateBindingLayout(binding);
				layouts.push_back(layout);
				pipelineEntry.layouts.push_back(m_shaderBindingLayouts.Insert(std::move(layout)));
			}

			if (pipelineDescriptor.usePerDrawModel)
			{
				layouts.push_back(m_modelLayout);
			}

			wgpu::PipelineLayoutDescriptor layoutDesc;
			layoutDesc.nextInChain = nullptr;
			layoutDesc.bindGroupLayoutCount = layouts.size();
			layoutDesc.bindGroupLayouts = layouts.data();
			wgpu::PipelineLayout pipelineLayout = m_device.CreatePipelineLayout(&layoutDesc);

			pipelineDesc.layout = pipelineLayout.Get();

			WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(m_device.Get(), &pipelineDesc);

			wgpuShaderModuleRelease(shaderModule);

			pipelineEntry.pipeline = wgpu::RenderPipeline::Acquire(pipeline);
			pipelineEntry.modelBinding = layouts.size() - 1;
			return m_pipelines.Insert(std::move(pipelineEntry));
		}

		ShaderBindingLayout GetPipelineShaderBindingLayout(const Pipeline pipeline, const size_t slot) override
		{
			auto& entry = m_pipelines[pipeline];
			return entry.layouts[slot];
		}

		virtual Texture CreateTexture(const ImageView image, TextureType type = TextureType::E2D) override
		{
			return CreateWGPUTexture(std::span<const ImageView>{&image, 1}, wgpu::TextureDimension::e2D, ConvertToWGPU(type));
		}

		virtual Texture CreateTexture(std::span<const ImageView> images, TextureType type = TextureType::Cube) override
		{
			return CreateWGPUTexture(images, wgpu::TextureDimension::e2D, ConvertToWGPU(type));
		}

		void ReleaseTexture(const Texture texture) override
		{
			m_textures.Remove(texture);
		}

		Sampler CreateSampler() override
		{
			wgpu::SamplerDescriptor samplerDesc;
			samplerDesc.nextInChain = nullptr;
			samplerDesc.addressModeU = wgpu::AddressMode::ClampToEdge;
			samplerDesc.addressModeV = wgpu::AddressMode::ClampToEdge;
			samplerDesc.addressModeW = wgpu::AddressMode::ClampToEdge;
			samplerDesc.magFilter = wgpu::FilterMode::Linear;
			samplerDesc.minFilter = wgpu::FilterMode::Linear;
			samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
			samplerDesc.lodMinClamp = 0.0f;
			samplerDesc.lodMaxClamp = 1.0f;
			samplerDesc.compare = wgpu::CompareFunction::Undefined;
			samplerDesc.maxAnisotropy = 1;
			return m_samplers.Insert(m_device.CreateSampler(&samplerDesc));
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
				case BufferType::Uniform:
					usage = WGPUBufferUsage_Uniform;
					break;
				case BufferType::Storage:
					usage = WGPUBufferUsage_Storage;
					break;
			}

			WGPUBuffer buffer = CreateWGPUBuffer(usage, bufferData, bufferSize);

			return {reinterpret_cast<U64>(buffer)};
		}

		Buffer CreateBuffer(BufferType bufferType, size_t bufferSize) override
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
			case BufferType::Uniform:
				usage = WGPUBufferUsage_Uniform;
				break;
			case BufferType::Storage:
				usage = WGPUBufferUsage_Storage;
				break;
			}

			WGPUBuffer buffer = CreateWGPUBuffer(usage, bufferSize);

			return {reinterpret_cast<U64>(buffer)};
		}

		virtual void ReleaseBuffer(Buffer buffer) override
		{
			wgpuBufferRelease(reinterpret_cast<WGPUBuffer>(buffer.value));
		}

		ShaderBinding CreateShaderBinding(ShaderBindingLayout layout, std::span<ShaderBindingEntryData> entries) override
		{
			std::vector<wgpu::BindGroupEntry> bindings;
			for (size_t i = 0; i < entries.size(); i++)
			{
				auto& binding = bindings.emplace_back();
				binding.nextInChain = nullptr;
				binding.binding = i;

				std::visit(overloaded
				{
					[&](const Buffer buffer)
					{
						binding.buffer = wgpu::Buffer(reinterpret_cast<WGPUBuffer>(buffer.value));
						binding.offset = 0;
						//TODO: size?
					},
					[&](const Texture texture)
					{
						auto& textures = m_textures;
						auto& entry = m_textures[texture];
						binding.textureView = entry.view;
					},
					[&](const Sampler sampler)
					{
						binding.sampler = m_samplers[sampler];
					}
				}, entries[i]);
			}

			wgpu::BindGroupDescriptor bindGroupDesc{};
			bindGroupDesc.nextInChain = nullptr;
			bindGroupDesc.layout = m_shaderBindingLayouts[layout];

			bindGroupDesc.entryCount = bindings.size();;
			bindGroupDesc.entries = bindings.data();
			auto shaderBinding = m_device.CreateBindGroup(&bindGroupDesc);
			return m_shaderBindings.Insert(std::move(shaderBinding));
		}

		void ReleaseShaderBinding(ShaderBinding binding) override
		{
			m_shaderBindings.Remove(binding);
		}

		//Hacks to get IMGUI working
		WGPUDevice GetDevice()
		{
			return m_device.Get();
		}

		WGPUTextureFormat GetSurfaceFormat()
		{
			return (WGPUTextureFormat) m_surfaceFormat;
		}

		WGPUTextureFormat GetDepthTextureFormat()
		{
			return m_depthTextureFormat;
		}

		WGPURenderPassEncoder GetRenderPass()
		{
			return m_renderPass;
		}

	private:
		WGPUTextureFormat m_depthTextureFormat = WGPUTextureFormat_Depth24Plus;
		WGPUTexture m_depthTexture;
		WGPUTextureView m_depthTextureView;
		wgpu::TextureFormat m_textureFormat = wgpu::TextureFormat::RGBA8Unorm;

		FrameState m_currentFrame;

		HandleVec<Texture, TextureEntry> m_textures;
		HandleVec<Sampler, wgpu::Sampler> m_samplers;
		HandleVec<Pipeline, PipelineEntry> m_pipelines;
		HandleVec<ShaderBindingLayout, wgpu::BindGroupLayout> m_shaderBindingLayouts;
		HandleVec<ShaderBinding, wgpu::BindGroup> m_shaderBindings;

		wgpu::BindGroupLayout m_modelLayout;
		InstanceBuffer m_instanceBuffer;
		size_t m_currentModelBinding = 0;

		WGPURenderPassEncoder m_renderPass;
		WGPUTextureView m_targetView;
		WGPUCommandEncoder m_encoder;

		bool m_initComplete = false; //TODO: tie into "await" system
		U32 m_width, m_height;
		wgpu::Queue m_queue;
		wgpu::Device m_device;
		wgpu::TextureFormat m_surfaceFormat;
		wgpu::Surface m_surface;
		wgpu::Instance m_instance;

		void RequestAdapterCallback(WGPURequestAdapterStatus status, WGPUAdapter adapterHandle, WLabel message)
		{
			if (status == WGPURequestAdapterStatus_Success)
			{
				auto adapter = wgpu::Adapter::Acquire(adapterHandle);

				#ifdef TAKO_EMSCRIPTEN
				m_surfaceFormat = m_surface.GetPreferredFormat(adapter);
				#else
				wgpu::SurfaceCapabilities capabilities;
				m_surface.GetCapabilities(adapter, &capabilities);
				for (int i = 0; i < capabilities.formatCount; i++)
				{
					LOG("supported format: {}", (uint32_t) capabilities.formats[i]);
				}
				m_surfaceFormat = capabilities.formats[0]; //Dawn workaround
				#endif

				LOG("Surface Format: {}", (uint32_t) m_surfaceFormat);

				wgpu::DeviceDescriptor deviceDesc;
				deviceDesc.nextInChain = nullptr;
				deviceDesc.label = "DefaultDevice";
				deviceDesc.requiredFeatureCount = 0;
				wgpu::RequiredLimits requiredLimits = GetRequiredLimits(adapter);
				deviceDesc.requiredLimits = &requiredLimits;
				deviceDesc.defaultQueue.nextInChain = nullptr;
				deviceDesc.defaultQueue.label = "DefaultQueue";
				#ifdef TAKO_EMSCRIPTEN
				deviceDesc.deviceLostCallback = DeviceLostCallback;
				#else
				deviceDesc.SetDeviceLostCallback(wgpu::CallbackMode::WaitAnyOnly, DeviceLostCallback);
				deviceDesc.SetUncapturedErrorCallback(UncapturedErrorCallback);
				#endif

				adapter.RequestDevice(
					&deviceDesc,
					[](WGPURequestDeviceStatus status, WGPUDevice device, WLabel message, void* pUserData)
					{
						reinterpret_cast<WebGPUContext*>(pUserData)->RequestDeviceCallback(status, device, message);
					},
					this
				);
			}
			else
			{
				LOG_ERR("Error requesting adapter: {}", message);
			}
		}

		void RequestDeviceCallback(WGPURequestDeviceStatus status, WGPUDevice device, WLabel message)
		{
			if (status == WGPURequestDeviceStatus_Success)
			{
				m_device = wgpu::Device::Acquire(device);
				#ifdef TAKO_EMSCRIPTEN
				wgpuDeviceSetUncapturedErrorCallback(m_device.Get(), UncapturedErrorCallback, nullptr);
				#endif

				m_queue = m_device.GetQueue();
				//wgpuQueueOnSubmittedWorkDone(m_queue, OnQueueWorkDone, this);


				ConfigureSurface();
				CreateDepthTexture();
				CreateModelLayout();
				CreateInstanceBuffer(1024);

				LOG("Renderer Setup Complete!");
				m_initComplete = true;
				/*
				wgpu::SupportedLimits supportedLimits;

				adapter.GetLimits(&supportedLimits);
				LOG("adapter.maxVertexAttributes: {}", supportedLimits.limits.maxVertexAttributes);

				m_device.GetLimits(&supportedLimits);
				LOG("device.maxVertexAttributes: {}", supportedLimits.limits.maxVertexAttributes);
				*/
			}
			else
			{
				LOG_ERR("Error requesting device: {}", message);
			}
		}

		static void OnQueueWorkDone(WGPUQueueWorkDoneStatus status, void* pUserData)
		{
			LOG("Work done ({})", fmt::underlying(status));
		}

#ifdef TAKO_EMSCRIPTEN
		static void DeviceLostCallback(WGPUDeviceLostReason reason, char const* message, void* pUserData)
#else
		static void DeviceLostCallback(const wgpu::Device& device, wgpu::DeviceLostReason reason, const char* message)
#endif
		{
			LOG_ERR("Device lost({}): {}", fmt::underlying(reason), message);
		}
#ifdef TAKO_EMSCRIPTEN
		static void UncapturedErrorCallback(WGPUErrorType type, char const* message, void* pUserData)
#else
		static void UncapturedErrorCallback(const wgpu::Device& device, wgpu::ErrorType type, const char* message)
#endif
		{
			LOG_ERR("Uncaptured device error({}): {}", fmt::underlying(type), message);
		}

		WGPUTextureView GetNextSurfaceTextureView()
		{
			wgpu::SurfaceTexture surfaceTexture;
			m_surface.GetCurrentTexture(&surfaceTexture);
			if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success)
			{
				return nullptr;
			}

			wgpu::TextureViewDescriptor viewDescriptor;
			viewDescriptor.nextInChain = nullptr;
			viewDescriptor.label = "Surface texture view";
			#ifndef TAKO_EMSCRIPTEN
			viewDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
			#endif
			viewDescriptor.format = surfaceTexture.texture.GetFormat();
			viewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
			viewDescriptor.baseMipLevel = 0;
			viewDescriptor.mipLevelCount = 1;
			viewDescriptor.baseArrayLayer = 0;
			viewDescriptor.arrayLayerCount = 1;
			viewDescriptor.aspect = wgpu::TextureAspect::All;
			WGPUTextureView targetView = surfaceTexture.texture.CreateView(&viewDescriptor).MoveToCHandle();

			return targetView;
		}

		Texture CreateWGPUTexture(const std::span<const ImageView> images, wgpu::TextureDimension dimension, wgpu::TextureViewDimension viewDimension)
		{
			ASSERT(images.size() > 0);
			// Assume all images are the same dimensions
			auto width = images[0].GetWidth();
			auto height = images[0].GetHeight();
			wgpu::TextureDescriptor textureDesc;
			textureDesc.nextInChain = nullptr;
			textureDesc.dimension = dimension;
			textureDesc.size = { width, height, static_cast<U32>(images.size()) };
			textureDesc.mipLevelCount = 1;
			textureDesc.sampleCount = 1;
			textureDesc.format = m_textureFormat;
			textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
			textureDesc.viewFormatCount = 0;
			textureDesc.viewFormats = nullptr;

			TextureEntry entry;
			entry.texture = m_device.CreateTexture(&textureDesc);
			ASSERT(entry.texture);

			for (U32 i = 0; i < images.size(); i++)
			{
				wgpu::ImageCopyTexture destination;
				#ifdef TAKO_EMSCRIPTEN
				destination.nextInChain = nullptr;
				#endif
				destination.texture = entry.texture;
				destination.mipLevel = 0;
				destination.origin = { 0, 0, i };
				destination.aspect = wgpu::TextureAspect::All;

				wgpu::TextureDataLayout source;
				source.nextInChain = nullptr;
				source.offset = 0;
				source.bytesPerRow = 4 * textureDesc.size.width;
				source.rowsPerImage = textureDesc.size.height;

				size_t imageSize = sizeof(Color) * width * height;
				auto writeSize = textureDesc.size;
				writeSize.depthOrArrayLayers = 1;
				m_queue.WriteTexture(&destination, images[i].GetData(), imageSize, &source, &writeSize);
			}

			wgpu::TextureViewDescriptor textureViewDesc {};
			textureViewDesc.nextInChain = nullptr;
			textureViewDesc.aspect = wgpu::TextureAspect::All;
			textureViewDesc.baseArrayLayer = 0;
			textureViewDesc.arrayLayerCount = images.size();
			textureViewDesc.baseMipLevel = 0;
			textureViewDesc.mipLevelCount = 1;
			textureViewDesc.dimension = viewDimension;
			textureViewDesc.format = m_textureFormat; //TODO: Get from texture?
			entry.view = entry.texture.CreateView(&textureViewDesc);
			ASSERT(entry.view);

			return m_textures.Insert(std::move(entry));
		}

		WGPUBuffer CreateWGPUBuffer(WGPUBufferUsage bufferType, const void* bufferData, size_t dataSize)
		{
			return CreateWGPUBuffer(bufferType, bufferData, dataSize, dataSize);
		}

		WGPUBuffer CreateWGPUBuffer(WGPUBufferUsage bufferType, const void* bufferData, size_t dataSize, size_t size)
		{
			WGPUBuffer buffer = CreateWGPUBuffer(bufferType, size);
			m_queue.WriteBuffer(buffer, 0, bufferData, dataSize);

			return buffer;
		}

		WGPUBuffer CreateWGPUBuffer(WGPUBufferUsage bufferType, size_t size)
		{
			WGPUBufferDescriptor bufferDesc{};
			bufferDesc.nextInChain = nullptr;
			bufferDesc.size = size;
			bufferDesc.usage = WGPUBufferUsage_CopyDst | bufferType;
			bufferDesc.mappedAtCreation = false;
			return wgpuDeviceCreateBuffer(m_device.Get(), &bufferDesc);
		}

		void ConfigureSurface()
		{
			wgpu::SurfaceConfiguration config;
			config.nextInChain = nullptr;
			config.width = m_width;
			config.height = m_height;
			config.format = m_surfaceFormat;
			config.viewFormatCount = 0;
			config.viewFormats = nullptr;
			config.usage = wgpu::TextureUsage::RenderAttachment;
			config.device = m_device;
			config.presentMode = wgpu::PresentMode::Fifo;
			config.alphaMode = wgpu::CompositeAlphaMode::Auto;

			m_surface.Configure(&config);
		}

		void CreateDepthTexture()
		{
			WGPUTextureDescriptor depthTextureDesc{};
			depthTextureDesc.nextInChain = nullptr;
			depthTextureDesc.dimension = WGPUTextureDimension_2D;
			depthTextureDesc.format = m_depthTextureFormat;
			depthTextureDesc.mipLevelCount = 1;
			depthTextureDesc.sampleCount = 1;
			depthTextureDesc.size = { m_width, m_height, 1};
			depthTextureDesc.usage = WGPUTextureUsage_RenderAttachment;
			depthTextureDesc.viewFormatCount = 1;
			depthTextureDesc.viewFormats = &m_depthTextureFormat;
			m_depthTexture = wgpuDeviceCreateTexture(m_device.Get(), &depthTextureDesc);

			WGPUTextureViewDescriptor depthTextureViewDesc{};
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

		void TerminateDepthTexture()
		{
			wgpuTextureViewRelease(m_depthTextureView);
			m_depthTextureView = nullptr;
			wgpuTextureDestroy(m_depthTexture);
			wgpuTextureRelease(m_depthTexture);
			m_depthTexture = nullptr;
		}

		wgpu::BindGroupLayout CreateBindingLayout(const ShaderBindingDescriptor& shaderBinding)
		{
			std::vector<wgpu::BindGroupLayoutEntry> bindingLayouts;
			for (int i = 0; i < shaderBinding.entries.size(); i++)
			{
				auto& binding = shaderBinding.entries[i];
				wgpu::BindGroupLayoutEntry bindingLayout;
				bindingLayout.binding = i;
				switch (binding.type)
				{
					case ShaderBindingType::Uniform:
					case ShaderBindingType::Storage:
						bindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
						bindingLayout.buffer.type = binding.type == ShaderBindingType::Uniform ? wgpu::BufferBindingType::Uniform : wgpu::BufferBindingType::ReadOnlyStorage;
						bindingLayout.buffer.minBindingSize = binding.size;
						break;
					case ShaderBindingType::Texture2D:
					case ShaderBindingType::TextureCube:
						bindingLayout.visibility = wgpu::ShaderStage::Fragment;
						bindingLayout.texture.sampleType = wgpu::TextureSampleType::Float;
						switch (binding.type)
						{
						case ShaderBindingType::Texture2D:
								bindingLayout.texture.viewDimension = wgpu::TextureViewDimension::e2D;
								break;
						case ShaderBindingType::TextureCube:
								bindingLayout.texture.viewDimension = wgpu::TextureViewDimension::Cube;
								break;
						}
						break;
					case ShaderBindingType::Sampler:
						bindingLayout.visibility = wgpu::ShaderStage::Fragment;
						bindingLayout.sampler.type = wgpu::SamplerBindingType::Filtering;
						break;
				}

				bindingLayouts.push_back(bindingLayout);
			}
			wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
			bindGroupLayoutDesc.nextInChain = nullptr;
			bindGroupLayoutDesc.entryCount = bindingLayouts.size();
			bindGroupLayoutDesc.entries = bindingLayouts.data();
			return m_device.CreateBindGroupLayout(&bindGroupLayoutDesc);
		}

		void CreateModelLayout()
		{
			wgpu::BindGroupLayoutEntry bindingLayout;
			bindingLayout.binding = 0;
			bindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
			bindingLayout.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
			bindingLayout.buffer.minBindingSize = sizeof(Matrix4);

			wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
			bindGroupLayoutDesc.nextInChain = nullptr;
			bindGroupLayoutDesc.entryCount = 1;
			bindGroupLayoutDesc.entries = &bindingLayout;
			m_modelLayout = m_device.CreateBindGroupLayout(&bindGroupLayoutDesc);
		}

		void CreateInstanceBuffer(size_t size)
		{
			InstanceBuffer ib;
			size_t s = size * sizeof(Matrix4);
			ib.buffer = wgpu::Buffer::Acquire(CreateWGPUBuffer(WGPUBufferUsage_Storage, s));

			wgpu::BindGroupEntry binding;
			binding.nextInChain = nullptr;
			binding.binding = 0;
			binding.buffer = ib.buffer;
			binding.offset = 0;
			binding.size = s;

			wgpu::BindGroupDescriptor bindGroupDesc{};
			bindGroupDesc.nextInChain = nullptr;
			bindGroupDesc.layout = m_modelLayout;

			bindGroupDesc.entryCount = 1;
			bindGroupDesc.entries = &binding;
			ib.group = m_device.CreateBindGroup(&bindGroupDesc);
			ib.size = size;
			m_instanceBuffer = ib;
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
			depthStencilState.depthWriteEnabled = WBool(false);
			depthStencilState.depthCompare = WGPUCompareFunction_Always;
			depthStencilState.stencilReadMask = 0xFFFFFFFF;
			depthStencilState.stencilWriteMask = 0xFFFFFFFF;
			depthStencilState.depthBias = 0;
			depthStencilState.depthBiasSlopeScale = 0;
			depthStencilState.depthBiasClamp = 0;
			SetDefault(depthStencilState.stencilFront);
			SetDefault(depthStencilState.stencilBack);
		}

		wgpu::RequiredLimits GetRequiredLimits(const wgpu::Adapter& adapter) const
		{
			wgpu::SupportedLimits supportedLimits;
			adapter.GetLimits(&supportedLimits);

			wgpu::RequiredLimits requiredLimits;

			requiredLimits.limits.maxVertexAttributes = 2;
			requiredLimits.limits.maxVertexBuffers = 1;
			requiredLimits.limits.maxBufferSize = 10000 * 11 * sizeof(float);
			requiredLimits.limits.maxVertexBufferArrayStride = 11 * sizeof(float);
			requiredLimits.limits.maxInterStageShaderComponents = 8;
			requiredLimits.limits.maxBindGroups = 4;
			requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
			requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
			requiredLimits.limits.maxTextureDimension1D = 2160;
			requiredLimits.limits.maxTextureDimension2D = 3840;
			requiredLimits.limits.maxTextureArrayLayers = 6;
			requiredLimits.limits.maxSampledTexturesPerShaderStage = 1;
			requiredLimits.limits.maxSamplersPerShaderStage = 1;

			requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
			requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;

			return requiredLimits;
		}
	};

	wgpu::TextureViewDimension ConvertToWGPU(TextureType type)
	{
		switch (type)
		{
			case TextureType::E2D: return wgpu::TextureViewDimension::e2D;
			case TextureType::Cube: return wgpu::TextureViewDimension::Cube;
		}
		ASSERT(false);
	}

}
