module;
#include "Utility.hpp"
#include "IGraphicsContext.hpp"
#include <RmlUi/Core/RenderInterface.h>
#include <vector>
export module Tako.RmlUi.Renderer;

import Tako.GraphicsContext;
import Tako.Math;
import Tako.Bitmap;

namespace tako
{

export class RmlUiRenderer : public Rml::RenderInterface
{
public:
	void Init(GraphicsContext* context)
	{
		m_context = context;
		InitPipeline();

		// Add white texture at index 0 to avoid creating a "no texture" pipeline
		Bitmap bmp(1, 1);
		bmp.Clear({255, 255, 255, 255});
		TexEntry tex;
		tex.tex = m_context->CreateTexture(bmp);
		tex.mat = m_context->CreateMaterial(&tex.tex);
		m_textures.push_back(tex);
	}

	void Begin()
	{
		m_context->BindPipeline(&m_pipeline);
		CameraUniformData camera;
		auto proj = Matrix4::ortho(0, m_context->GetWidth(), m_context->GetHeight(), 0, -1000, 1000);
		camera.proj = proj;
		m_context->UpdateCamera(camera);
		Vector4 uniform;
		m_context->UpdateUniform(&uniform, sizeof(Vector4));
		m_zBuffer = 0;
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

		m_compiledGeometry.push_back(geom);
		return m_compiledGeometry.size();
	}

	void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override
	{
		auto& geom = m_compiledGeometry[geometry - 1];
		auto& tex = m_textures[texture];
		m_context->BindVertexBuffer(&geom.vertices);
		m_context->BindIndexBuffer(&geom.indices);
		m_context->BindMaterial(&tex.mat);

		Matrix4 trans = Matrix4::translation(translation.x, translation.y, m_zBuffer++);
		m_context->DrawIndexed(geom.count, trans);
	}

	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override
	{
		LOG("TODO: ReleaseGeometry");
	}

	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override
	{
		LOG("TODO: LoadTexture");
		return 0;
	}

	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override
	{
		//TODO: Implement "BitmapView" to avoid copy
		Bitmap bmp(reinterpret_cast<const Color*>(source.data()), source_dimensions.x, source_dimensions.y);
		TexEntry tex;
		tex.tex = m_context->CreateTexture(bmp);
		tex.mat = m_context->CreateMaterial(&tex.tex);

		m_textures.push_back(tex);
		return m_textures.size() - 1;
	}

	void ReleaseTexture(Rml::TextureHandle texture) override
	{
		LOG("TODO: ReleaseTexture");
	}

	void EnableScissorRegion(bool enable) override
	{
		LOG("TODO: EnableScissorRegion");
	}

	void SetScissorRegion(Rml::Rectanglei region) override
	{
		LOG("TODO: SetScissorRegion");
	}

private:
	struct CompiledGeometry
	{
		Buffer vertices;
		Buffer indices;
		size_t count;
	};

	struct TexEntry
	{
		Texture tex;
		Material mat;
	};

	GraphicsContext* m_context;
	Pipeline m_pipeline;
	size_t m_zBuffer;
	std::vector<CompiledGeometry> m_compiledGeometry;
	std::vector<TexEntry> m_textures;

	void InitPipeline()
	{
		const char* shaderSource = R"(
			struct Camera
			{
				view: mat4x4f,
				projection: mat4x4f,
			};

			struct Lighting
			{
				lightPos: vec4f,
			};

			@group(0) @binding(0) var<uniform> camera: Camera;

			//@group(1) @binding(0) var<uniform> lighting: Lighting;

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
				out.position = camera.projection * model * vec4(in.position, 0.0, 1.0);
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

		pipelineDescriptor.pipelineUniformSize = sizeof(Vector4);

		pipelineDescriptor.pushConstants = nullptr;
		pipelineDescriptor.pushConstantsSize = 0;

		m_pipeline = m_context->CreatePipeline(pipelineDescriptor);
	}
};

}
