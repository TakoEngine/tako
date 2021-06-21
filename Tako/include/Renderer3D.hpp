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

	class Renderer3D
	{
	public:
		Renderer3D(GraphicsContext* context);

		void Begin();
		void End();

		void DrawMesh(const Mesh& mesh, const Matrix4& model);
		void DrawCube(const Matrix4& model);
		Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
	protected:
		GraphicsContext* m_context;
		Mesh m_cubeMesh;
	};
}
