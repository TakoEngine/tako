module;
#include "IGraphicsContext.hpp"
#include "VertexBuffer.hpp"
#include "Pipeline.hpp"
#include "Utility.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "../tinyobjloader/tiny_obj_loader.h" //TODO: why
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <array>
export module Tako.Renderer3D;

import Tako.FileSystem;
import Tako.GraphicsContext;

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
		void DrawModel(const Model& model, const Matrix4& transform);
		void DrawModelInstanced(const Model& model, size_t instanceCount, const Matrix4* transforms);
		Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);

		void SetCameraView(const Matrix4& view);
		void SetLightPosition(Vector3 lightPos);

		Model LoadModel(const char* file);
		Mesh LoadMesh(const char* file);
		Texture CreateTexture(const Bitmap& bitmap);
	protected:
		GraphicsContext* m_context;
		Mesh m_cubeMesh;
		Pipeline m_pipeline;
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

	struct UniformBufferObject
	{
		Matrix4 model;
		Matrix4 view;
		Matrix4 proj;
	};

	struct LightSettings
	{
		Vector3 lightPos;
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
		const char* vertPath = "/../shader/shader.vert.spv";
		const char* fragPath = "/../shader/shader.frag.spv";

		auto vertCode = LoadShaderCode(vertPath);
		auto fragCode = LoadShaderCode(fragPath);

		std::array<PipelineVectorAttribute, 4> vertexAttributes = { PipelineVectorAttribute::Vec3, PipelineVectorAttribute::Vec3, PipelineVectorAttribute::Vec3, PipelineVectorAttribute::Vec2 };
		size_t pushConstant = sizeof(Matrix4);

		PipelineDescriptor pipelineDescriptor;
		pipelineDescriptor.vertCode = vertCode.data();
		pipelineDescriptor.vertSize = vertCode.size();
		pipelineDescriptor.fragCode = fragCode.data();
		pipelineDescriptor.fragSize = fragCode.size();
		pipelineDescriptor.vertexAttributes = vertexAttributes.data();
		pipelineDescriptor.vertexAttributeSize = vertexAttributes.size();

		pipelineDescriptor.pipelineUniformSize = sizeof(LightSettings);

		pipelineDescriptor.pushConstants = &pushConstant;
		pipelineDescriptor.pushConstantsSize = 0;

		m_pipeline = m_context->CreatePipeline(pipelineDescriptor);

		m_cubeMesh = CreateMesh(cubeVertices, cubeIndices);
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
		CameraUniformData cam;
		cam.view = view;
		cam.proj = Matrix4::perspective(45, m_context->GetWidth() / (float) m_context->GetHeight(), 1, 1000);
		cam.viewProj = cam.proj * cam.view;
		m_context->UpdateCamera(cam);
	}

	void Renderer3D::SetLightPosition(Vector3 lightPos)
	{
		m_context->UpdateUniform(&lightPos, sizeof(LightSettings));
	}

	Model Renderer3D::LoadModel(const char* file)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType |
			aiProcess_FlipUVs);

		std::vector<Texture> textures;
		textures.resize(scene->mNumTextures);
		for (int t = 0; t < scene->mNumTextures; t++)
		{
			auto tex = scene->mTextures[t];
			if (tex->mHeight > 0)
			{
				Bitmap bmp(tex->mWidth, tex->mHeight);
				for (int i = 0; i < tex->mWidth * tex->mHeight; i++)
				{
					auto& tx = tex->pcData[i];
					bmp.GetData()[i] = { tx.r, tx.g, tx.b, tx.a };
				}
				textures[t] = m_context->CreateTexture(bmp);
			}
			else
			{
				textures[t] = m_context->CreateTexture(Bitmap::FromFileData(reinterpret_cast<const U8*>(tex->pcData), tex->mWidth));
			}
		}

		std::vector<Material> materials;
		materials.resize(scene->mNumMeshes);

		for (int matIndex = 0; matIndex < scene->mNumMaterials; matIndex++)
		{
			auto mat = scene->mMaterials[matIndex];
			auto diffuseTexCount = mat->GetTextureCount(aiTextureType_DIFFUSE);
			if (diffuseTexCount > 0)
			{
				aiString texPath;
				mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
				materials[matIndex] = { m_context->CreateMaterial(&textures[(size_t)atoi(&texPath.C_Str()[1])]) };
			}
			//TODO: handle more cases
		}

		std::vector<Mesh> meshes;
		meshes.resize(scene->mNumMeshes);

		std::vector<Node> nodes;
		nodes.resize(scene->mNumMeshes);

		for (int mIndex = 0; mIndex < scene->mNumMeshes; mIndex++)
		{
			auto mesh = scene->mMeshes[mIndex];
			std::vector<Vertex> vertices;
			vertices.resize(mesh->mNumVertices);
			for (int v = 0; v < mesh->mNumVertices; v++)
			{
				Vertex vert;

				vert.pos = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z };
				vert.normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
				if (mesh->HasVertexColors(0))
				{
					vert.color = { mesh->mColors[0][v].r, mesh->mColors[0][v].g, mesh->mColors[0][v].b };
					LOG("{} {} {}", vert.color.x, vert.color.y, vert.color.z);
				}
				else
				{
					vert.color = { 1, 1, 1 };
				}
				if (mesh->GetNumUVChannels() >= 1)
				{
					vert.uv = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
				}
				else
				{
					vert.uv = { 0, 0 };
				}
				vertices[v] = vert;
			}

			std::vector<U16> indices;
			indices.resize(mesh->mNumFaces * 3);
			for (int f = 0; f < mesh->mNumFaces; f++)
			{
				indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
				indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
				indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
			}

			auto finalMesh = CreateMesh(vertices, indices);
			meshes[mIndex] = finalMesh;

			nodes[mIndex] = { finalMesh, materials[mesh->mMaterialIndex]};
		}

		/*
		for (int m = 0; m < scene->mNumMaterials; m++)
		{
			auto mat = scene->mMaterials[m];
			LOG("{}", scene->mMaterials[m]->GetName().C_Str());
			LOG("{}", scene->mMaterials[m]->mNumProperties);
			auto diffuseTexCount = scene->mMaterials[m]->GetTextureCount(aiTextureType_DIFFUSE);
			for (int t = 0; t < diffuseTexCount; t++)
			{
				aiString texPath;
				scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, t, &texPath);
				LOG("{}", texPath.C_Str());
			}
			auto baseTexCount = scene->mMaterials[m]->GetTextureCount(aiTextureType_BASE_COLOR);
			for (int t = 0; t < baseTexCount; t++)
			{
				aiString texPath;
				scene->mMaterials[m]->GetTexture(aiTextureType_BASE_COLOR, t, &texPath);
				LOG("{}", texPath.C_Str());
			}
			for (int p = 0; p < scene->mMaterials[m]->mNumProperties; p++)
			{
				auto prop = scene->mMaterials[m]->mProperties[p];
				switch (prop->mType)
				{

				case aiPTI_String:
					LOG("{} {}", prop->mKey.C_Str(), ((aiString*)prop->mData)->C_Str());
					break;
				case aiPTI_Float:
					LOG("{} {} {}", prop->mKey.C_Str(), prop->mDataLength, *((float*)prop->mData));
				}
			}
		}
		*/


		return { meshes, materials, textures, nodes };
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

	Texture Renderer3D::CreateTexture(const Bitmap& bitmap)
	{
		return m_context->CreateTexture(bitmap);
	}
}
