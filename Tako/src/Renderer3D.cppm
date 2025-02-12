module;
#include "IGraphicsContext.hpp"
#include "VertexBuffer.hpp"
#include "Pipeline.hpp"
#include "Utility.hpp"
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include "../tinyobjloader/tiny_obj_loader.h" //TODO: why
/*
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
*/
#include <vector>
#include <array>
#include <algorithm>
export module Tako.Renderer3D;

//import fastgltf;
import Tako.StringView;
import Tako.FileSystem;
import Tako.GraphicsContext;


template <>
struct fastgltf::ElementTraits<tako::Vector2> : fastgltf::ElementTraitsBase<tako::Vector2, AccessorType::Vec2, float> {};

template <>
struct fastgltf::ElementTraits<tako::Vector3> : fastgltf::ElementTraitsBase<tako::Vector3, AccessorType::Vec3, float> {};

namespace tako
{
	struct Vertex
	{
		Vector3 pos;
		Vector3 normal;
		Vector3 color;
		Vector2 uv;

		constexpr bool operator==(const Vertex& other) const
		{
			return
				pos == other.pos &&
				normal == other.normal &&
				color == other.color &&
				uv == other.uv;
		}
	};

	struct CameraUniformData
	{
		Matrix4 view;
		Matrix4 proj;
		//Matrix4 viewProj;
	};

	export using Material = ShaderBinding;

	struct MaterialDescriptor
	{
		TextureType textureType = TextureType::E2D;
	};

	export struct Mesh
	{
		Buffer vertexBuffer;
		Buffer indexBuffer;
		uint16_t indexCount;
	};

	export struct Node
	{
		Mesh mesh;
		Material mat;
	};

	export struct Model
	{
		std::vector<Mesh> meshes;
		std::vector<Material> materials;
		std::vector<Texture> textures;
		std::vector<Node> nodes;
	};

	export struct Light
	{
		Vector3 position;
		Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	};

	export using Skybox = ShaderBinding;

	export class Renderer3D
	{
	public:
		Renderer3D(GraphicsContext* context);

		void Begin();
		void End();

		void DrawMesh(const Mesh& mesh, const Material& material, const Matrix4& model);
		void DrawMeshInstanced(const Mesh& mesh, const Material& material, size_t instanceCount, const Matrix4* transforms);
		void DrawCube(const Matrix4& model, const Material& material);
		void DrawCubeInstanced(const Material& material, size_t instanceCount, const Matrix4* transforms);
		void DrawModel(const Model& model, const Matrix4& transform);
		void DrawModelInstanced(const Model& model, size_t instanceCount, const Matrix4* transforms);
		Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);

		void SetCameraView(const Matrix4& view);
		void SetLights(std::span<Light> lights);

		Model LoadModel(StringView file);
		Mesh LoadMesh(const char* file);

		Texture CreateTexture(const ImageView image)
		{
			return m_context->CreateTexture(image);
		}

		Texture CreateTexture(const std::span<const ImageView> images)
		{
			return m_context->CreateTexture(images);
		}

		Texture CreateTexture(const std::span<const Bitmap, 6> bitmaps)
		{
			std::array<ImageView, 6> images;
			for (int i = 0; i < bitmaps.size(); i++)
			{
				images[i] = bitmaps[i];
			}
			return m_context->CreateTexture(images);
		}

		Material CreateMaterial(Texture texture, const MaterialDescriptor& materialDescriptor = {})
		{
			std::array<ShaderBindingEntryData, 2> bindingData
			{{
				texture,
				m_sampler,
			}};
			auto binding = m_context->CreateShaderBinding(m_context->GetPipelineShaderBindingLayout(m_pipeline, 2), bindingData);
			return binding;
		}

		Skybox CreateSkybox(Texture cubemap);
		void RenderSkybox(Skybox skybox);
	protected:
		GraphicsContext* m_context;
		Sampler m_sampler;
		Mesh m_cubeMesh;
		Pipeline m_pipeline;
		Buffer m_cameraBuffer;
		ShaderBinding m_cameraBinding;
		Buffer m_lightSettingsBuffer;
		Buffer m_lightBuffer;
		size_t m_lightBufferSize = 0;
		ShaderBinding m_lightBinding;
		Pipeline m_skyPipeline;
		Texture m_skyTexture;
		Material m_skyMaterial;
		Buffer m_skyCameraBuffer;
		ShaderBinding m_skyCameraBinding;

	private:
		CameraUniformData m_cameraData;
		void CreatePipeline();
		void CreateLightBuffer(size_t size);
		void CreateSkyboxPipeline();
	};
}


namespace tako
{
	const std::vector<Vertex> cubeVertices =
	{
		{{-1, -1, -1}, {}, {1.0f, 0.0f, 0.0f}, {}},
		{{ 1, -1, -1}, {}, {0.0f, 1.0f, 0.0f}, {}},
		{{ 1,  1, -1}, {}, {0.0f, 0.0f, 1.0f}, {}},
		{{-1,  1, -1}, {}, {1.0f, 1.0f, 0.0f}, {}},
		{{-1, -1,  1}, {}, {0.0f, 1.0f, 1.0f}, {}},
		{{ 1, -1,  1}, {}, {1.0f, 0.0f, 1.0f}, {}},
		{{ 1,  1,  1}, {}, {0.0f, 0.0f, 0.0f}, {}},
		{{-1,  1,  1}, {}, {1.0f, 1.0f, 1.0f}, {}},
	};

	const std::vector<uint16_t> cubeIndices =
	{
		0, 1, 3, 3, 1, 2,
		1, 5, 2, 2, 5, 6,
		5, 4, 6, 6, 4, 7,
		4, 0, 7, 7, 0, 3,
		3, 2, 7, 7, 2, 6,
		4, 5, 0, 0, 5, 1
	};

	struct GPULight
	{
		Vector4 positionVS;
		Vector4 color;
	};

	struct LightSettings
	{
		Vector4 ambient;
		U32 lightCount;
		U32 padding[3];
	};

	std::vector<U8> LoadShaderCode(const char* codePath)
	{
		auto path = FileSystem::GetExecutablePath() + codePath;
		size_t fileSize = FileSystem::GetFileSize(path.c_str());
		std::vector<U8> code(fileSize);
		size_t bytesRead = 0;
		bool readSuccess = FileSystem::ReadFile(path.c_str(), code.data(), code.size(), bytesRead);
		ASSERT(readSuccess && fileSize == bytesRead);

		return code;
	}

	Renderer3D::Renderer3D(GraphicsContext* context) : m_context(context)
	{
		CreatePipeline();
		m_sampler = m_context->CreateSampler();
		m_cameraBuffer = m_context->CreateBuffer(BufferType::Uniform, sizeof(CameraUniformData));
		std::array<ShaderBindingEntryData, 1> cameraBindingData
		{{
				m_cameraBuffer,
		}};
		m_cameraBinding = m_context->CreateShaderBinding(m_context->GetPipelineShaderBindingLayout(m_pipeline, 0), cameraBindingData);

		m_lightSettingsBuffer = m_context->CreateBuffer(BufferType::Uniform, sizeof(LightSettings));
		CreateLightBuffer(1);

		CreateSkyboxPipeline();
		m_skyCameraBuffer = m_context->CreateBuffer(BufferType::Uniform, sizeof(Matrix4));
		std::array<ShaderBindingEntryData, 1> skyCameraBindingData
		{{
			m_skyCameraBuffer,
		}};
		m_skyCameraBinding = m_context->CreateShaderBinding(m_context->GetPipelineShaderBindingLayout(m_skyPipeline, 0), skyCameraBindingData);

		m_cubeMesh = CreateMesh(cubeVertices, cubeIndices);
	}

	void Renderer3D::CreatePipeline()
	{
		const char* shaderSource = R"(
			struct Camera
			{
				view: mat4x4f,
				projection: mat4x4f,
			};

			struct Light
			{
				positionVS: vec4f,
				color: vec4f
			};

			struct Lighting
			{
				ambient: vec4f,
				lightCount: u32,
			};

			@group(0) @binding(0) var<uniform> camera: Camera;

			@group(1) @binding(0) var<uniform> lighting: Lighting;
			@group(1) @binding(1) var<storage, read> lights: array<Light>;

			@group(2) @binding(0) var baseColorTexture: texture_2d<f32>;
			@group(2) @binding(1) var baseColorSampler: sampler;

			@group(3) @binding(0) var<storage, read> models : array<mat4x4f>;

			struct VertexInput {
				@location(0) position: vec3f,
				@location(1) normal: vec3f,
				@location(2) color: vec3f,
				@location(3) uv: vec2f,
			};

			struct VertexOutput {
				@builtin(position) position: vec4f,
				@location(0) color: vec3f,
				@location(1) uv: vec2f,
				@location(2) positionVS: vec3f,
				@location(3) normalVS: vec3f,
			}

			@vertex
			fn vs_main(in: VertexInput, @builtin(instance_index) instanceIndex: u32) -> VertexOutput {
				let model = models[instanceIndex];
				let modelView = camera.view * model;
				var out: VertexOutput;
				out.position = camera.projection * modelView * vec4f(in.position, 1.0);
				out.color = in.color;
				out.uv = in.uv;

				out.positionVS = (modelView * vec4f(in.position, 1.0)).xyz;
				out.normalVS = (modelView * vec4f(in.normal, 0)).xyz;

				return out;
			}

			@fragment
			fn fs_main(in: VertexOutput) -> @location(0) vec4f {
				let eyePos = vec4f(0, 0, 0, 1);
				let baseColor = textureSample(baseColorTexture, baseColorSampler, in.uv);
				let N = normalize(vec4f(in.normalVS, 1));
				let P = vec4f(in.positionVS, 1);

				let V = normalize(eyePos - P);
				var shading = lighting.ambient;
				for (var i : u32 = 0; i < lighting.lightCount; i += 1) {
					let light = lights[i];
					var L = light.positionVS - P;
					let distance = length(L);
					L = L / distance;
					shading += max(dot(N, L), 0) * light.color;
				}
				//let color =
				//	baseColor * vec3f(0.1,0.1,0.1) +
				//	baseColor * vec3f(1,1,1) * 200 * cosTheta / (dist*dist) +
				//	vec3f(0.3,0.3,0.3) * vec3f(1,1,1) * 200 * pow(cosAlpha, 5) / (dist*dist);

				let color = baseColor * shading;
				//let color = baseColor;

				return color;
			}
		)";

		std::array vertexAttributes = { PipelineVectorAttribute::Vec3, PipelineVectorAttribute::Vec3, PipelineVectorAttribute::Vec3, PipelineVectorAttribute::Vec2 };

		PipelineDescriptor pipelineDescriptor;
		pipelineDescriptor.shaderCode = shaderSource;
		pipelineDescriptor.vertEntry = "vs_main";
		pipelineDescriptor.fragEntry = "fs_main";
		pipelineDescriptor.vertexAttributes = vertexAttributes.data();
		pipelineDescriptor.vertexAttributeSize = vertexAttributes.size();

		std::array<ShaderEntry, 1> cameraBinding
		{
			{ShaderBindingType::Uniform, sizeof(CameraUniformData)}
		};
		std::array<ShaderEntry, 2> lightingBinding
		{{
			ShaderBindingType::Uniform, sizeof(LightSettings),
			ShaderBindingType::Storage, sizeof(GPULight)
		}};
		std::array<ShaderEntry, 2> materialBinding
		{{
			ShaderBindingType::Texture2D, 0,
			ShaderBindingType::Sampler, 0
		}};
		std::array<ShaderBindingDescriptor, 3> shaderBindings
		{{
			cameraBinding,
			lightingBinding,
			materialBinding
		}};
		pipelineDescriptor.shaderBindings = shaderBindings;

		m_pipeline = m_context->CreatePipeline(pipelineDescriptor);
		LOG("Created pipeline");
	}

	void Renderer3D::CreateLightBuffer(size_t size)
	{
		if (m_lightBuffer.value)
		{
			m_context->ReleaseBuffer(m_lightBuffer);
		}
		m_lightBuffer = m_context->CreateBuffer(BufferType::Storage, size * sizeof(GPULight));
		m_lightBufferSize = size;
		std::array<ShaderBindingEntryData, 2> lightingBindingData
		{{
			m_lightSettingsBuffer,
			m_lightBuffer,
		}};
		if (m_lightBinding.value)
		{
			m_context->ReleaseShaderBinding(m_lightBinding);
		}
		m_lightBinding = m_context->CreateShaderBinding(m_context->GetPipelineShaderBindingLayout(m_pipeline, 1), lightingBindingData);
	}

	void Renderer3D::CreateSkyboxPipeline()
	{
		const char* shaderSource = R"(
			struct Camera
			{
				viewDirectionProjectionInverse: mat4x4f,
			};

			@group(0) @binding(0) var<uniform> camera: Camera;

			@group(1) @binding(0) var skyTex: texture_cube<f32>;
			@group(1) @binding(1) var skySampler: sampler;

			struct VertexOutput {
				@builtin(position) position: vec4f,
				@location(0) pos: vec4f,
			}

			@vertex
			fn vs_main (@builtin(vertex_index) vertexIndex: u32) -> VertexOutput {
				let pos = array(
					vec2f(-1, 3),
					vec2f(-1,-1),
					vec2f( 3,-1),
				);
				var out: VertexOutput;
				out.position = vec4f(pos[vertexIndex], 1, 1);
				out.pos = out.position;
				return out;
			}

			@fragment
			fn fs_main(in: VertexOutput) -> @location(0) vec4f {
				let t = camera.viewDirectionProjectionInverse * in.pos;
				return textureSample(skyTex, skySampler, normalize(t.xyz / t.w) * vec3f(1, 1, -1)); // z -1 necessary?
			}
		)";

		PipelineDescriptor pipelineDescriptor;
		pipelineDescriptor.shaderCode = shaderSource;
		pipelineDescriptor.vertEntry = "vs_main";
		pipelineDescriptor.fragEntry = "fs_main";

		std::array<ShaderEntry, 1> cameraBinding
		{
			{ShaderBindingType::Uniform, sizeof(Matrix4)}
		};
		std::array<ShaderEntry, 2> materialBinding
		{{
			ShaderBindingType::TextureCube, 0,
			ShaderBindingType::Sampler, 0
		}};
		std::array<ShaderBindingDescriptor, 2> shaderBindings
		{{
			cameraBinding,
			materialBinding
		}};
		pipelineDescriptor.shaderBindings = shaderBindings;

		pipelineDescriptor.usePerDrawModel = false;

		m_skyPipeline = m_context->CreatePipeline(pipelineDescriptor);
	}

	Skybox Renderer3D::CreateSkybox(Texture cubemap)
	{
		std::array<ShaderBindingEntryData, 2> bindingData
		{{
			cubemap,
			m_sampler,
		}};
		auto binding = m_context->CreateShaderBinding(m_context->GetPipelineShaderBindingLayout(m_skyPipeline, 1), bindingData);
		return binding;
	}

	void Renderer3D::RenderSkybox(const Skybox skybox)
	{
		m_context->BindPipeline(&m_skyPipeline);
		Matrix4 viewProjectionInverse = Matrix4::inverse(m_cameraData.proj * m_cameraData.view);
		m_context->UpdateBuffer(m_skyCameraBuffer, &viewProjectionInverse, sizeof(viewProjectionInverse));
		m_context->Bind(m_skyCameraBinding, 0);
		m_context->Bind(skybox, 1);

		m_context->Draw(3);
	}

	void Renderer3D::Begin()
	{
		m_context->BindPipeline(&m_pipeline);
		m_context->Bind(m_cameraBinding, 0);
		m_context->Bind(m_lightBinding, 1);
	}

	void Renderer3D::End()
	{
	}

	void Renderer3D::DrawMesh(const Mesh& mesh, const Material& material, const Matrix4& model)
	{
		m_context->BindVertexBuffer(&mesh.vertexBuffer);
		m_context->BindIndexBuffer(&mesh.indexBuffer);
		m_context->Bind(material, 2);
		m_context->DrawIndexed(mesh.indexCount, model);
	}

	void Renderer3D::DrawMeshInstanced(const Mesh& mesh, const Material& material, size_t instanceCount, const Matrix4* transforms)
	{
		m_context->BindVertexBuffer(&mesh.vertexBuffer);
		m_context->BindIndexBuffer(&mesh.indexBuffer);
		m_context->Bind(material, 2);
		m_context->DrawIndexed(mesh.indexCount, instanceCount, transforms);
	}

	void Renderer3D::DrawCube(const Matrix4& model, const Material& material)
	{
		DrawMesh(m_cubeMesh, material, model);
	}

	void Renderer3D::DrawCubeInstanced(const Material& material, size_t instanceCount, const Matrix4* transforms)
	{
		DrawMeshInstanced(m_cubeMesh, material, instanceCount, transforms);
	}

	void Renderer3D::DrawModel(const Model& model, const Matrix4& transform)
	{
		for (const Node& node : model.nodes)
		{
			DrawMesh(node.mesh, node.mat, transform);
		}
	}

	void Renderer3D::DrawModelInstanced(const Model& model, size_t instanceCount, const Matrix4* transforms)
	{
		for (const Node& node : model.nodes)
		{
			DrawMeshInstanced(node.mesh, node.mat, instanceCount, transforms);
		}
	}

	Mesh Renderer3D::CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices)
	{
		Mesh mesh;
		mesh.vertexBuffer = m_context->CreateBuffer(BufferType::Vertex, vertices.data(), sizeof(vertices[0]) * vertices.size());
		mesh.indexBuffer = m_context->CreateBuffer(BufferType::Index, indices.data(), sizeof(indices[0]) * indices.size());
		mesh.indexCount = indices.size();
		return std::move(mesh);
	}

	void Renderer3D::SetCameraView(const Matrix4& view)
	{
		m_cameraData.view = view;
		m_cameraData.proj = Matrix4::perspective(45, m_context->GetWidth() / (float) m_context->GetHeight(), 0.1, 1000);
		//cam.viewProj = cam.proj * cam.view;
		m_context->UpdateBuffer(m_cameraBuffer, &m_cameraData, sizeof(m_cameraData));
	}

	void Renderer3D::SetLights(::std::span<Light> lights)
	{
		std::vector<GPULight> gpuLights;
		gpuLights.reserve(lights.size());

		for (auto& light : lights)
		{
			GPULight& gpuLight = gpuLights.emplace_back();
			gpuLight.positionVS = m_cameraData.view * Vector4(light.position);
			gpuLight.color = light.color;
		}

		if (gpuLights.size() > 0)
		{
			if (gpuLights.size() > m_lightBufferSize)
			{
				CreateLightBuffer(gpuLights.size());
			}
			m_context->UpdateBuffer(m_lightBuffer, gpuLights.data(), gpuLights.size() * sizeof(GPULight));
		}

		LightSettings settings;
		settings.ambient = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
		settings.lightCount = gpuLights.size();
		m_context->UpdateBuffer(m_lightSettingsBuffer, &settings, sizeof(settings));
	}

	Model Renderer3D::LoadModel(StringView file)
	{
		Model model;
		fastgltf::Parser parser;
		std::filesystem::path path(file.ToStringView());
		auto data = fastgltf::GltfDataBuffer::FromPath(path);;
		if( data.error() != fastgltf::Error::None)
		{
			LOG_ERR("gltf data error: {}", fastgltf::getErrorName(data.error()));
			return {};
		}

		auto assetRes = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::None);
		if (assetRes.error() != fastgltf::Error::None)
		{
			LOG_ERR("gltf asset error: {}", fastgltf::getErrorName(assetRes.error()));
			return {};
		}

		auto& asset = assetRes.get();

		size_t imageCount = asset.images.size();
		model.textures.resize(imageCount);
		for (size_t i = 0; i < imageCount; i++)
		{
			auto& rawImage = asset.images[i];
			std::visit(fastgltf::visitor
			{
				[&](fastgltf::sources::URI& filePath)
				{
					LOG("{}", filePath.uri.string());
				},
				[&](fastgltf::sources::Vector& vector)
				{
					LOG("vector");
				},
				[&](fastgltf::sources::BufferView& view)
				{
					auto& bufferView = asset.bufferViews[view.bufferViewIndex];
					auto& buffer = asset.buffers[bufferView.bufferIndex];
					std::visit(fastgltf::visitor
					{
						[&](fastgltf::sources::URI& uri)
						{
							LOG("BufferView uri: {}", uri.uri.string());
						},
						[&](fastgltf::sources::Array& arr)
						{
							Bitmap img = Bitmap::FromFileData(reinterpret_cast<U8*>(arr.bytes.data()) + bufferView.byteOffset, bufferView.byteLength);
							model.textures[i] = CreateTexture(img);
						},
						[&](fastgltf::sources::Vector& vec)
						{
							LOG("BufferView vec: {}", vec.bytes.size());
						},
						[](auto& arg) {}
					}, buffer.data);
				},
				[](auto& arg) { LOG("default"); }
			}, rawImage.data);
		}

		size_t materialCount = asset.materials.size();
		model.materials.resize(materialCount);
		for (size_t i = 0; i < materialCount; i++)
		{
			auto& mat = asset.materials[i];
			// Only support materials with texture for now
			ASSERT(mat.pbrData.baseColorTexture.has_value());
			size_t texIndex = mat.pbrData.baseColorTexture.value().textureIndex;
			size_t imageIndex = asset.textures[texIndex].imageIndex.value();
			model.materials[i] = CreateMaterial(model.textures[imageIndex]);
		}

		fastgltf::iterateSceneNodes(asset, 0, fastgltf::math::fmat4x4(), [&](fastgltf::Node& node, fastgltf::math::fmat4x4 matrix)
		{
			if (node.meshIndex.has_value())
			{
				std::vector<Vertex> vertices;
				std::vector<U16> indices;
				LOG("{}", node.name);
				auto& rawMesh = asset.meshes[node.meshIndex.value()];
				for (auto& primitive : rawMesh.primitives)
				{
					//TODO: merge primitives if same material?
					auto& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];
					indices.reserve(indexAccessor.count);
					fastgltf::iterateAccessor<U32>(asset, indexAccessor, [&](U32 index)
					{
						indices.push_back(index);
					});


					auto& posAccessor = asset.accessors[primitive.findAttribute("POSITION")->accessorIndex];
					vertices.resize(posAccessor.count);
					fastgltf::iterateAccessorWithIndex<Vector3>(asset, posAccessor, [&](Vector3 pos, size_t index)
					{
						Vertex v;
						v.pos = pos;
						//v.pos.y *= -1;
						v.color = Vector3(1, 1, 1);
						vertices[index] = v;
					});

					auto normals = primitive.findAttribute("NORMAL");
					if (normals != primitive.attributes.end())
					{
						auto& normalAccessor = asset.accessors[normals->accessorIndex];
						fastgltf::iterateAccessorWithIndex<Vector3>(asset, normalAccessor, [&](Vector3 normal, size_t index)
						{
							vertices[index].normal = normal;
						});
					}

					auto colors = primitive.findAttribute("COLOR_0");
					if (normals != primitive.attributes.end())
					{
						auto& colorAccessor = asset.accessors[colors->accessorIndex];
						fastgltf::iterateAccessorWithIndex<Vector3>(asset, colorAccessor, [&](Vector3 color, size_t index)
						{
							vertices[index].color = color;
						});
					}

					auto uvs = primitive.findAttribute("TEXCOORD_0");
					if (uvs != primitive.attributes.end())
					{
						auto& uvAccessor = asset.accessors[uvs->accessorIndex];
						fastgltf::iterateAccessorWithIndex<Vector2>(asset, uvAccessor, [&](Vector2 uv, size_t index)
						{
							vertices[index].uv = uv;
						});
					}

					Node node;
					node.mesh = CreateMesh(vertices, indices);
					if (primitive.materialIndex.has_value())
					{
						auto materialIndex = primitive.materialIndex.value();
						node.mat = model.materials[materialIndex];
					}
					model.nodes.push_back(node);
				}
			}
		});

		return model;
	}

	Mesh Renderer3D::LoadMesh(const char* file)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn;
		std::string err;

		auto success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file, nullptr);
		if (!warn.empty())
		{
			LOG_WARN("{}", warn);
		}

		if (!err.empty())
		{
			LOG_ERR("{}", err);
		}
		ASSERT(success);


		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;
		for (size_t s = 0; s < shapes.size(); s++) {
			size_t indexOffset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				int fv = 3; //Triangles per face
				for (size_t v = 0; v < fv; v++) {
					tinyobj::index_t idx = shapes[s].mesh.indices[indexOffset + v];

					//vertex position
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					//vertex normal
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

					tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
					tinyobj::real_t uy = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];

					//copy it into our vertex
					Vertex new_vert
					{
						//pos
						{
							vx, vy, vz
						},
						//normal
						{
							nx, ny, nz
						},
						//color
						{
							nx, ny, nz
						},
						//uv
						{
							ux, uy
						}
					};

					//TODO: use map
					auto find = std::find(vertices.begin(), vertices.end(), new_vert);
					if (find == std::end(vertices))
					{
						indices.push_back(vertices.size());
						vertices.push_back(new_vert);
					}
					else
					{
						indices.push_back(std::distance(vertices.begin(), find));
					}


				}
				indexOffset += fv;
			}
		}

		return CreateMesh(vertices, indices);
	}
}
