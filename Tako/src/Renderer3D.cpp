#include "Renderer3D.hpp"

namespace tako
{
	const std::vector<Vertex> cubeVertices =
	{
		{{-1, -1, -1}, {1.0f, 0.0f, 0.0f}},
		{{ 1, -1, -1}, {0.0f, 1.0f, 0.0f}},
		{{ 1,  1, -1}, {0.0f, 0.0f, 1.0f}},
		{{-1,  1, -1}, {1.0f, 1.0f, 0.0f}},
		{{-1, -1,  1}, {0.0f, 1.0f, 1.0f}},
		{{ 1, -1,  1}, {1.0f, 0.0f, 1.0f}},
		{{ 1,  1,  1}, {0.0f, 0.0f, 0.0f}},
		{{-1,  1,  1}, {1.0f, 1.0f, 1.0f}},
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

	Renderer3D::Renderer3D(GraphicsContext* context) : m_context(context)
	{
		m_cubeMesh = CreateMesh(cubeVertices, cubeIndices);
	}

	void Renderer3D::Begin()
	{
		m_context->Begin();
	}

	void Renderer3D::End()
	{
		m_context->End();
	}

	void Renderer3D::DrawMesh(const Mesh& mesh, const Matrix4& model)
	{
		m_context->BindVertexBuffer(&mesh.vertexBuffer);
		m_context->BindIndexBuffer(&mesh.indexBuffer);
		m_context->DrawIndexed(mesh.indexCount, model);
	}

	void Renderer3D::DrawCube(const Matrix4& model)
	{
		DrawMesh(m_cubeMesh, model);
	}

	Mesh Renderer3D::CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices)
	{
		Mesh mesh;
		mesh.vertexBuffer = m_context->CreateBuffer(BufferType::Vertex, vertices.data(), sizeof(vertices[0]) * vertices.size());
		mesh.indexBuffer = m_context->CreateBuffer(BufferType::Index, indices.data(), sizeof(indices[0]) * indices.size());
		mesh.indexCount = indices.size();
		return std::move(mesh);
	}
}
