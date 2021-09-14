#pragma once
#include "GraphicsContext.hpp"
#include "VertexBuffer.hpp"

namespace tako
{

	struct Mesh
	{
		Buffer vertexBuffer;
		Buffer indexBuffer;
		uint16_t indexCount;
	};

	struct Node
	{
		Mesh mesh;
		Material mat;
	};

	struct Model
	{
		std::vector<Mesh> meshes;
		std::vector<Material> materials;
		std::vector<Texture> textures;
		std::vector<Node> nodes;
	};

	class Renderer3D
	{
	public:
		Renderer3D(GraphicsContext* context);

		void Begin();
		void End();

		void DrawMesh(const Mesh& mesh, const Material& material, const Matrix4& model);
		void DrawCube(const Matrix4& model, const Material& material);
		void DrawModel(const Model& model, const Matrix4& transform);
		Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);

		void SetCameraView(const Matrix4& view);

		Model LoadModel(const char* file);
		Mesh LoadMesh(const char* file);
		Texture CreateTexture(const Bitmap& bitmap);
	protected:
		GraphicsContext* m_context;
		Mesh m_cubeMesh;
	};
}
