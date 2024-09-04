#include "PixelArtDrawer.hpp"
#include <array>
#include "Utility.hpp"

import Tako.Assets;

namespace tako
{
	struct ImageVertex
	{
		Vector2 position;
		Vector2 texcoord;
	};
	namespace
	{
		constexpr Vector2 vertices[] =
		{
			{ 0, 0},
			{ 1, 0},
			{ 1, 1},
			{ 0, 0},
			{ 1, 1},
			{ 0, 1}
		};



		constexpr std::array<ImageVertex, 6> CreateImageVertices(float x, float y, float w, float h)
		{
			return
			{ {
					{ {0, 0}, {x    , y    } },
					{ {1, 0}, {x + w, y    } },
					{ {1, 1}, {x + w, y + h } },
					{ {0, 0}, {x    , y    }} ,
					{ {1, 1}, {x + w, y + h} },
					{ {0, 1}, {x    , y + h} }
			} };
		}

		constexpr std::array<ImageVertex, 6> imageVertices = CreateImageVertices(0, 0, 1, 1);

		std::vector<U8> LoadShaderCode(const char* codePath)
		{
			size_t fileSize = Assets::GetAssetFileSize(codePath);
			std::vector<U8> code(fileSize);
			size_t bytesRead = 0;
			bool readSuccess = Assets::ReadAssetFile(codePath, code.data(), code.size(), bytesRead);
			ASSERT(readSuccess && fileSize == bytesRead);

			return code;
		}

		struct QuadUniformObject
		{
			Matrix4 projection;
			Vector4 color;
		};
	}

	PixelArtDrawer::PixelArtDrawer(GraphicsContext* context) : m_context(context)
	{
		m_scale = 1;
		m_sizeSet = false;
		/*
		glClearColor(0, 0, 0, 1);
		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		*/

		//Quad Pipeline
		{
			const char* vertPath = "/quad.vert.spv";
			const char* fragPath = "/quad.frag.spv";

			auto vertCode = LoadShaderCode(vertPath);
			auto fragCode = LoadShaderCode(fragPath);

			std::array<PipelineVectorAttribute, 1> vertexAttributes = { PipelineVectorAttribute::Vec2 };
			size_t pushConstant = sizeof(Matrix4);

			PipelineDescriptor pipelineDescriptor;
			pipelineDescriptor.vertCode = vertCode.data();
			pipelineDescriptor.vertSize = vertCode.size();
			pipelineDescriptor.fragCode = fragCode.data();
			pipelineDescriptor.fragSize = fragCode.size();
			pipelineDescriptor.vertexAttributes = vertexAttributes.data();
			pipelineDescriptor.vertexAttributeSize = vertexAttributes.size();
			pipelineDescriptor.pushConstants = &pushConstant;
			pipelineDescriptor.pushConstantsSize = 1;

			m_quadProgram = m_context->CreatePipeline(pipelineDescriptor);

			m_quadVBO = m_context->CreateBuffer(BufferType::Vertex, vertices, sizeof(vertices));
			uint16_t indices[] = { 0, 1, 2, 3, 4, 5 };
			m_quadVBOIndex = m_context->CreateBuffer(BufferType::Index, indices, sizeof(indices));
		}

		//Image Pipeline
		{
			const char* vertPath = "/image.vert.spv";
			const char* fragPath = "/image.frag.spv";

			auto vertCode = LoadShaderCode(vertPath);
			auto fragCode = LoadShaderCode(fragPath);

			std::array<PipelineVectorAttribute, 2> vertexAttributes = { PipelineVectorAttribute::Vec2, PipelineVectorAttribute::Vec2 };
			size_t pushConstant = sizeof(Matrix4);

			PipelineDescriptor pipelineDescriptor;
			pipelineDescriptor.vertCode = vertCode.data();
			pipelineDescriptor.vertSize = vertCode.size();
			pipelineDescriptor.fragCode = fragCode.data();
			pipelineDescriptor.fragSize = fragCode.size();
			pipelineDescriptor.vertexAttributes = vertexAttributes.data();
			pipelineDescriptor.vertexAttributeSize = vertexAttributes.size();
			pipelineDescriptor.pushConstants = &pushConstant;
			pipelineDescriptor.pushConstantsSize = 1;

			m_imageProgram = m_context->CreatePipeline(pipelineDescriptor);

			m_imageVBO = m_context->CreateBuffer(BufferType::Vertex, imageVertices.data(), sizeof(imageVertices));
			uint16_t indices[] = { 0, 1, 2, 3, 4, 5 };
			m_imageVBOIndex = m_context->CreateBuffer(BufferType::Index, indices, sizeof(indices));
		}

		Resize(1024, 768);
	}

	PixelArtDrawer::~PixelArtDrawer()
	{

	}

	void PixelArtDrawer::Begin()
	{
		m_context->Begin();
	}

	void PixelArtDrawer::End()
	{
		m_context->End();
	}

	void PixelArtDrawer::SetClearColor(Color c)
	{
		m_clearColor = c;
		/*
		if (!m_sizeSet)
		{
			glClearColor(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
		}
		*/
	}

	void PixelArtDrawer::SetTargetSize(int width, int height)
	{
		m_sizeW = width;
		m_sizeH = height;

		if (m_sizeSet || m_autoScale)
		{
			CalculateScale();
		}
		if (m_sizeSet)
		{
			//glClearColor(0, 0, 0, 1);
		}
	}

	void PixelArtDrawer::AutoScale()
	{
		m_autoScale = true;
		CalculateScale();
	}

	void PixelArtDrawer::SetCameraPosition(Vector2 position)
	{
		m_cameraPosition = position;
	}

	Vector2 PixelArtDrawer::GetCameraPosition()
	{
		return m_cameraPosition;
	}

	Vector2 PixelArtDrawer::GetCameraViewSize()
	{
		return
		{
			static_cast<float>(m_width) / m_scale,
			static_cast<float>(m_height) / m_scale
		};
	}

	void PixelArtDrawer::Clear()
	{
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (m_sizeSet)
		{
			DrawRectangle(0, 0, m_sizeW, m_sizeH, m_clearColor);
		}
	}

	void PixelArtDrawer::DrawRectangle(float x, float y, float w, float h, Color c)
	{
		GetDrawOffset(x, y, w, h);
		m_context->BindPipeline(&m_quadProgram);

		QuadUniformObject ubo;
		ubo.projection = Matrix4::transpose(Matrix4::ortho(0, 1024, 0, 768, -1, 1));
		Matrix4 model = Matrix4::transpose(Matrix4::translation(x, y, 0)) * Matrix4::ScaleMatrix(w, h, 1);

		ubo.color = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
		m_context->UpdateUniform(&ubo, sizeof(QuadUniformObject));

		m_context->BindVertexBuffer(&m_quadVBO);
		m_context->BindIndexBuffer(&m_quadVBOIndex);
		m_context->DrawIndexed(6, model);
	}

	void PixelArtDrawer::DrawImage(float x, float y, float w, float h, const TextureHandle img, Color color)
	{
		DrawTextureQuad(x, y, w, h, img, m_imageVBO, color);
	}

	void PixelArtDrawer::DrawSprite(float x, float y, float w, float h, const Sprite* sprite, Color color)
	{

	}

	void PixelArtDrawer::DrawTextureQuad(float x, float y, float w, float h, const TextureHandle texture, Buffer buffer, Color color)
	{
		GetDrawOffset(x, y, w, h);
		m_context->BindPipeline(&m_imageProgram);
		Matrix4 projection = Matrix4::transpose(Matrix4::ortho(0, 1024, 0, 768, -1, 1));
		Matrix4 model = Matrix4::transpose(Matrix4::translation(x, y, 0)) * Matrix4::ScaleMatrix(w, h, 1);

		m_context->UpdateUniform(&projection, sizeof(Matrix4));
		m_context->BindVertexBuffer(&m_imageVBO);
		m_context->BindIndexBuffer(&m_imageVBOIndex);
		auto matSearch = m_texMatMap.find(texture.value);
		if (matSearch == m_texMatMap.end())
		{
			Texture tex = { texture, 42, 42 }; //Create dummy texture, needs a refactor
			auto material = m_context->CreateMaterial(&tex);
			m_texMatMap[texture.value] = material;
			m_context->BindMaterial(&material);
		}
		else
		{
			m_context->BindMaterial(&matSearch->second);
		}
		
		m_context->DrawIndexed(6, model);
	}

	void PixelArtDrawer::Resize(int w, int h)
	{
		m_width = w;
		m_height = h;
		if (m_sizeSet || m_autoScale)
		{
			CalculateScale();
		}
		/*
		glViewport(0, 0, w, h);

		auto err = glGetError();
		if (err != GL_NO_ERROR)
		{
			LOG("error preresize");
		}


		Matrix4 ortho = Matrix4::transpose(Matrix4::ortho(0, w, h, 0, 0, 100));
		glUseProgram(m_quadProgram);
		glUniformMatrix4fv(m_quadProjectionUniform, 1, GL_FALSE, &ortho[0]);
		glUseProgram(m_imageProgram);
		glUniformMatrix4fv(m_imageProjectionUniform, 1, GL_FALSE, &ortho[0]);
		err = glGetError();
		if (err != GL_NO_ERROR)
		{
			LOG("error resize");
		}
		*/
	}

	Texture PixelArtDrawer::CreateTexture(const Bitmap& bitmap)
	{
		return m_context->CreateTexture(bitmap);
	}

	Sprite* PixelArtDrawer::CreateSprite(const Texture texture, float x, float y, float w, float h)
	{
		return nullptr;
	}

	void PixelArtDrawer::UpdateTexture(Texture texture, const Bitmap& bitmap)
	{

	}

	void PixelArtDrawer::GetDrawOffset(float& x, float& y, float& w, float& h)
	{
		auto extents = GetCameraViewSize() / 2;
		x = x - m_cameraPosition.x + extents.x;
		y = m_cameraPosition.y - y + extents.y;

		x = x * m_scale;
		y = y * m_scale;
		w = w * m_scale;
		h = h * m_scale;

		if (m_sizeSet)
		{
			x += (m_width - m_sizeW * m_scale) / 2;
			y += (m_height - m_sizeH * m_scale) / 2;
		}
	}

	void PixelArtDrawer::CalculateScale()
	{
		int wScale = m_width / m_sizeW;
		int hScale = m_height / m_sizeH;
		m_scale = std::max(1, std::min(wScale, hScale));
		LOG("SCALE {}", m_scale);
	}
}
