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
		void SetLightPosition(Vector3 lightPos);

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

		Material CreateSkybox(Texture cubemap);
		void RenderSkybox(Material skybox);
	protected:
		GraphicsContext* m_context;
		Mesh m_cubeMesh;
		Pipeline m_pipeline;
		Pipeline m_skyPipeline;
		Texture m_skyTexture;
		Material m_skyMaterial;

	private:
		CameraUniformData m_cameraData;
		void CreatePipeline();
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

	struct LightSettings
	{
		Vector4 lightPos;
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
		CreateSkyboxPipeline();

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

			struct Lighting
			{
				lightPos: vec4f,
			};

			@group(0) @binding(0) var<uniform> camera: Camera;

			@group(1) @binding(0) var<uniform> lighting: Lighting;

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
				@location(1) normal: vec3f,
				@location(2) uv: vec2f,
				@location(3) positionWorld: vec3f,
				@location(4) normalCamera: vec3f,
				@location(5) eyeDirection: vec3f,
				@location(6) lightDirection: vec3f,
			}

			@vertex
			fn vs_main(in: VertexInput, @builtin(instance_index) instanceIndex: u32) -> VertexOutput {
				let model = models[instanceIndex];
				var out: VertexOutput;
				out.position = camera.projection * camera.view * model * vec4f(in.position, 1.0);
				out.color = in.color;
				out.normal = (model * vec4f(in.normal, 0.0)).xyz;
				out.uv = in.uv;

				out.positionWorld = (model * vec4f(in.position, 1)).xyz;

				let vertexPosCamera = (camera.view * model * vec4f(in.position,1)).xyz;
				out.eyeDirection = vec3f(0,0,0) - vertexPosCamera;

				let lightPosCamera = (camera.view * lighting.lightPos).xyz;
				out.lightDirection = lightPosCamera + out.eyeDirection;

				out.normalCamera = ( camera.view * model * vec4f(in.normal, 0)).xyz;

				return out;
			}

			@fragment
			fn fs_main(in: VertexOutput) -> @location(0) vec4f {
				let normal = normalize(in.normal);
				let dist = length(lighting.lightPos.xyz - in.positionWorld);
				let n = normalize(in.normalCamera);
				let l = normalize(in.lightDirection);
				let cosTheta = clamp(dot(n,l), 0, 1);

				let E = normalize(in.eyeDirection);
				let R = reflect(-l, n);
				let cosAlpha = clamp( dot(E,R), 0, 1);

				let baseColor = textureSample(baseColorTexture, baseColorSampler, in.uv).rgb;
				let color =
					baseColor * vec3f(0.1,0.1,0.1) +
					baseColor * vec3f(1,1,1) * 200 * cosTheta / (dist*dist) +
					vec3f(0.3,0.3,0.3) * vec3f(1,1,1) * 200 * pow(cosAlpha, 5) / (dist*dist);

				//let color = baseColor * shading;

				return vec4f(color, 1.0);
			}
		)";

		std::array<PipelineVectorAttribute, 4> vertexAttributes = { PipelineVectorAttribute::Vec3, PipelineVectorAttribute::Vec3, PipelineVectorAttribute::Vec3, PipelineVectorAttribute::Vec2 };

		PipelineDescriptor pipelineDescriptor;
		pipelineDescriptor.shaderCode = shaderSource;
		pipelineDescriptor.vertEntry = "vs_main";
		pipelineDescriptor.fragEntry = "fs_main";
		pipelineDescriptor.vertexAttributes = vertexAttributes.data();
		pipelineDescriptor.vertexAttributeSize = vertexAttributes.size();

		pipelineDescriptor.pipelineUniformSize = sizeof(LightSettings);

		m_pipeline = m_context->CreatePipeline(pipelineDescriptor);
	}

	void Renderer3D::CreateSkyboxPipeline()
	{
		const char* shaderSource = R"(
			struct Camera
			{
				viewDirectionProjectionInverse: mat4x4f,
				projection: mat4x4f,
			};

			@group(0) @binding(0) var<uniform> camera: Camera;

			@group(2) @binding(0) var skyTex: texture_cube<f32>;
			@group(2) @binding(1) var skySampler: sampler;

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

		pipelineDescriptor.pipelineUniformSize = sizeof(LightSettings);
		pipelineDescriptor.samplerTextureType = TextureType::Cube;

		pipelineDescriptor.usePerDrawModel = false;

		m_skyPipeline = m_context->CreatePipeline(pipelineDescriptor);
	}

	Material Renderer3D::CreateSkybox(Texture cubemap)
	{
		return m_context->CreateMaterial(cubemap, { TextureType::Cube });
	}

	void Renderer3D::RenderSkybox(Material skybox)
	{
		m_context->BindPipeline(&m_skyPipeline);
		CameraUniformData cam;
		cam.view = Matrix4::inverse(m_cameraData.proj * m_cameraData.view);
		m_context->UpdateCamera(cam);
		m_context->BindMaterial(&skybox);

		m_context->Draw(3);
	}

	void Renderer3D::Begin()
	{
		m_context->BindPipeline(&m_pipeline);
	}

	void Renderer3D::End()
	{
	}

	void Renderer3D::DrawMesh(const Mesh& mesh, const Material& material, const Matrix4& model)
	{
		m_context->BindVertexBuffer(&mesh.vertexBuffer);
		m_context->BindIndexBuffer(&mesh.indexBuffer);
		m_context->BindMaterial(&material);
		m_context->DrawIndexed(mesh.indexCount, model);
	}

	void Renderer3D::DrawMeshInstanced(const Mesh& mesh, const Material& material, size_t instanceCount, const Matrix4* transforms)
	{
		m_context->BindVertexBuffer(&mesh.vertexBuffer);
		m_context->BindIndexBuffer(&mesh.indexBuffer);
		m_context->BindMaterial(&material);
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
		m_context->UpdateCamera(m_cameraData);
	}

	void Renderer3D::SetLightPosition(Vector3 lightPos)
	{
		LightSettings lights;
		lights.lightPos = Vector4(lightPos);
		m_context->UpdateUniform(&lights, sizeof(LightSettings));
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
			model.materials[i] = m_context->CreateMaterial(model.textures[imageIndex]);
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
						v.pos.y *= -1;
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
