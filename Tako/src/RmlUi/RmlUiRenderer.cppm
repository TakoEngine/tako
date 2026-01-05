module;
#include "Utility.hpp"
#include "IGraphicsContext.hpp"
#include <RmlUi/Core/RenderInterface.h>
#include <vector>
export module Tako.RmlUi.Renderer;

import Tako.GraphicsContext;
import Tako.Math;
import Tako.Bitmap;
import Tako.Resources;

namespace tako
{

export class RmlUiRenderer : public Rml::RenderInterface
{
public:
	void Init(GraphicsContext* context, Resources* resources)
	{
		m_context = context;
		m_resources = resources;
		InitPipeline();
		m_sampler = context->CreateSampler();
		m_transformLayout = context->GetPipelineShaderBindingLayout(m_pipeline, 1);
		m_texLayout = context->GetPipelineShaderBindingLayout(m_pipeline, 2);
		// Add white texture at index 0 to avoid creating a "no texture" pipeline
		Bitmap bmp(1, 1);
		bmp.Clear({255, 255, 255, 255});
		TexEntry tex;
		tex.tex = m_context->CreateTexture(bmp);
		tex.mat = CreateTextureBinding(tex.tex);
		m_textures.push_back(tex);

		m_cameraBuffer = m_context->CreateBuffer(BufferType::Uniform, sizeof(Matrix4));
		std::array<ShaderBindingEntryData, 1> cameraBindingData
		{{
			m_cameraBuffer,
		}};
		m_cameraBinding = m_context->CreateShaderBinding(m_context->GetPipelineShaderBindingLayout(m_pipeline, 0), cameraBindingData);

		auto& defaultTransform = CreateTransformBinding();
		auto identity = Matrix4::identity;
		m_context->UpdateBuffer(defaultTransform.buffer, &identity[0], sizeof(identity));
	}

	void Begin()
	{
		m_context->BindPipeline(&m_pipeline);
		auto proj = Matrix4::ortho(0, m_context->GetWidth(), m_context->GetHeight(), 0, -1000, 1000);
		m_context->UpdateBuffer(m_cameraBuffer, &proj, sizeof(proj));
		m_context->Bind(m_cameraBinding, 0);
		m_context->Bind(m_transformEntries[0].binding, 1);
		m_zBuffer = 0;
		m_transformCurrentIndex = 1;
	}

	void End()
	{
	}

	Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override
	{
		CompiledGeometry geom;
		geom.vertices = m_context->CreateBuffer(BufferType::Vertex, vertices.data(), vertices.size() * sizeof(Rml::Vertex));
		std::vector<U16> ind(indices.size());
		for (int i = 0; i < indices.size(); i++)
		{
			ind[i] = indices[i];
		}
		geom.indices = m_context->CreateBuffer(BufferType::Index, ind.data(), ind.size() * sizeof(U16));
		geom.count = indices.size();

		if (m_compiledGeometryFreelist)
		{
			auto handle = m_compiledGeometryFreelist;
			auto g = &m_compiledGeometry[m_compiledGeometryFreelist - 1];
			m_compiledGeometryFreelist = g->count;
			*g = geom;
			return handle;
		}

		m_compiledGeometry.push_back(geom);
		return m_compiledGeometry.size();
	}

	void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override
	{
		auto& geom = m_compiledGeometry[geometry - 1];
		auto& tex = m_textures[texture];
		m_context->BindVertexBuffer(&geom.vertices);
		m_context->BindIndexBuffer(&geom.indices);
		m_context->Bind(tex.mat, 2);

		Matrix4 trans = Matrix4::translation(translation.x, translation.y, m_zBuffer++);
		m_context->DrawIndexed(geom.count, trans);
	}

	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override
	{
		auto& geom = m_compiledGeometry[geometry - 1];
		m_context->ReleaseBuffer(geom.vertices);
		m_context->ReleaseBuffer(geom.indices);

		if (m_compiledGeometryFreelist)
		{
			geom.count = m_compiledGeometryFreelist;
		}
		else
		{
			geom.count = 0;
		}
		m_compiledGeometryFreelist = geometry;
	}

	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override
	{
		Texture texture = m_resources->Load<Texture>(source);
		return MakeTextureHandle(texture, true);
	}

	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override
	{
		ImageView img(reinterpret_cast<const Color*>(source.data()), source_dimensions.x, source_dimensions.y);
		Texture texture = m_context->CreateTexture(img);
		return MakeTextureHandle(texture, false);
	}

	Rml::TextureHandle MakeTextureHandle(Texture texture, bool loadedFromResources)
	{
		TexEntry tex;
		tex.tex = texture;
		tex.mat = CreateTextureBinding(tex.tex);
		tex.loadedFromResources = loadedFromResources;

		if (m_textureFreelist)
		{
			auto handle = m_textureFreelist;
			auto t = &m_textures[m_textureFreelist];
			m_textureFreelist = *reinterpret_cast<size_t*>(t);
			*t = tex;
			return handle;
		}

		m_textures.push_back(tex);
		return m_textures.size() - 1;
	}

	void ReleaseTexture(Rml::TextureHandle texture) override
	{
		LOG("Release Texture");
		auto tex = &m_textures[texture];
		m_context->ReleaseShaderBinding(tex->mat);
		if (tex->loadedFromResources)
		{
			m_resources->Release(tex->tex);
		}
		else
		{
			m_context->ReleaseTexture(tex->tex);
		}

		*reinterpret_cast<size_t*>(tex) = m_textureFreelist;
		m_textureFreelist = texture;
	}

	void EnableScissorRegion(bool enable) override
	{
		LOG("TODO: EnableScissorRegion");
	}

	void SetScissorRegion(Rml::Rectanglei region) override
	{
		LOG("TODO: SetScissorRegion");
	}

	void SetTransform(const Rml::Matrix4f* transform) override
	{
		if (transform == nullptr)
		{
			m_context->Bind(m_transformEntries[0].binding, 1);
			return;
		}

		if (m_transformCurrentIndex >= m_transformEntries.size())
		{
			CreateTransformBinding();
		}
		m_context->UpdateBuffer(m_transformEntries[m_transformCurrentIndex].buffer, transform, sizeof(Matrix4));
		m_context->Bind(m_transformEntries[m_transformCurrentIndex].binding, 1);
		m_transformCurrentIndex++;
	}

private:
	struct CompiledGeometry
	{
		Buffer vertices;
		Buffer indices;
		size_t count; // Also used for freelist
	};

	struct TexEntry
	{
		Texture tex;
		ShaderBinding mat;
		bool loadedFromResources;
	};

	struct TransformEntry
	{
		Buffer buffer;
		ShaderBinding binding;
	};

	GraphicsContext* m_context;
	Resources* m_resources;
	Pipeline m_pipeline;
	Sampler m_sampler;
	Buffer m_cameraBuffer;
	ShaderBinding m_cameraBinding;
	ShaderBindingLayout m_texLayout;
	ShaderBindingLayout m_transformLayout;
	size_t m_zBuffer;
	std::vector<CompiledGeometry> m_compiledGeometry;
	size_t m_compiledGeometryFreelist = 0;
	std::vector<TexEntry> m_textures;
	size_t m_textureFreelist = 0;
	std::vector<TransformEntry> m_transformEntries;
	size_t m_transformCurrentIndex = 1;

	void InitPipeline()
	{
		const char* shaderSource = R"(
			struct Camera
			{
				projection: mat4x4f,
			};

			@group(0) @binding(0) var<uniform> camera: Camera;

			@group(1) @binding(0) var<uniform> rmlTransform: mat4x4f;

			@group(2) @binding(0) var baseColorTexture: texture_2d<f32>;
			@group(2) @binding(1) var baseColorSampler: sampler;

			@group(3) @binding(0) var<storage, read> models : array<mat4x4f>;

			struct VertexInput {
				@location(0) position: vec2f,
				@location(1) color: vec4f,
				@location(2) uv: vec2f,
			};

			struct VertexOutput {
				@builtin(position) position: vec4f,
				@location(0) color: vec4f,
				@location(2) uv: vec2f,
			}

			@vertex
			fn vs_main(in: VertexInput, @builtin(instance_index) instanceIndex: u32) -> VertexOutput {
				let model = models[instanceIndex];
				var out: VertexOutput;
				out.position = camera.projection * rmlTransform * model * vec4(in.position, 0.0, 1.0);
				out.color = in.color;
				out.uv = in.uv;

				return out;
			}

			@fragment
			fn fs_main(in: VertexOutput) -> @location(0) vec4f {
				let baseColor = textureSample(baseColorTexture, baseColorSampler, in.uv);
				let color = in.color * baseColor;

				return color;
			}
		)";

		std::array vertexAttributes = { PipelineVectorAttribute::Vec2, PipelineVectorAttribute::RGBA, PipelineVectorAttribute::Vec2 };

		PipelineDescriptor pipelineDescriptor;
		pipelineDescriptor.shaderCode = shaderSource;
		pipelineDescriptor.vertEntry = "vs_main";
		pipelineDescriptor.fragEntry = "fs_main";
		pipelineDescriptor.vertexAttributes = vertexAttributes.data();
		pipelineDescriptor.vertexAttributeSize = vertexAttributes.size();

		std::array<ShaderEntry, 1> cameraBinding
		{{
			ShaderBindingType::Uniform, sizeof(Matrix4)
		}};
		std::array<ShaderEntry, 1> transformBinding
		{{
			ShaderBindingType::Uniform, sizeof(Matrix4)
		}};
		std::array<ShaderEntry, 2> materialBinding
		{{
			ShaderBindingType::Texture2D, 0,
			ShaderBindingType::Sampler, 0
		}};
		std::array<ShaderBindingDescriptor, 3> shaderBindings
		{{
			cameraBinding,
			transformBinding,
			materialBinding
		}};
		pipelineDescriptor.shaderBindings = shaderBindings;

		m_pipeline = m_context->CreatePipeline(pipelineDescriptor);
	}

	ShaderBinding CreateTextureBinding(Texture tex)
	{
		std::array<ShaderBindingEntryData, 2> bindings
		{{
			tex,
			m_sampler
		}};
		return m_context->CreateShaderBinding(m_texLayout, bindings);
	}

	TransformEntry& CreateTransformBinding()
	{
		TransformEntry entry;
		entry.buffer = m_context->CreateBuffer(BufferType::Uniform, sizeof(Matrix4));
		std::array<ShaderBindingEntryData, 1> bindingData
		{{
			entry.buffer
		}};
		entry.binding = m_context->CreateShaderBinding(m_transformLayout, bindingData);
		m_transformEntries.push_back(entry);
		return m_transformEntries.back();
	}
};

}
