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

	struct Material
	{
		Texture texture;
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

		void DrawMesh(const Mesh& mesh, const Texture& texture, const Matrix4& model);
		void DrawCube(const Matrix4& model, const Texture& texture);
		void DrawModel(const Model& model, const Matrix4& transform);
		Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);

		Model LoadModel(const char* file);
		Mesh LoadMesh(const char* file);
		Texture CreateTexture(const Bitmap& bitmap);
	protected:
		GraphicsContext* m_context;
		Mesh m_cubeMesh;
	};
}
