#pragma once

#include "Texture.hpp"
#include "Sprite.hpp"
#include "VertexBuffer.hpp"
#include "Pipeline.hpp"
#include <unordered_map>
#include <string>

import Tako.Math;
import Tako.GraphicsContext;
import Tako.Bitmap;

namespace tako
{
	class PixelArtDrawer
	{
	public:
		PixelArtDrawer(GraphicsContext* context);
		~PixelArtDrawer();

		void Begin();
		void End();

		void SetClearColor(Color c);
		void SetTargetSize(int width, int height);
		void AutoScale();
		void SetCameraPosition(Vector2 position);
		Vector2 GetCameraPosition();
		Vector2 GetCameraViewSize();

		void Clear();
		void DrawRectangle(float x, float y, float w, float h, Color c);
		void DrawImage(float x, float y, float w, float h, const Texture img, Color color = {255, 255, 255, 255});
		void DrawSprite(float x, float y, float w, float h, const Sprite* sprite, Color color = {255, 255, 255, 255});

		Texture CreateTexture(const Bitmap& bitmap);
		Sprite* CreateSprite(const Texture texture, float x, float y, float w, float h);

		void UpdateTexture(Texture texture, const Bitmap& bitmap);

		void Resize(int w, int h);
	protected:
		GraphicsContext* m_context;
	private:
		void DrawTextureQuad(float x, float y, float w, float h, const Texture texture, Buffer buffer, Color color);
		void GetDrawOffset(float& x, float& y, float& w, float& h);
		void CalculateScale();
		std::unordered_map<U64, Texture> m_texMatMap;

		Vector2 m_cameraPosition;
		U32 m_width;
		U32 m_height;

		bool m_sizeSet;
		bool m_autoScale;
		U32 m_scale;
		U32 m_sizeW;
		U32 m_sizeH;
		Color m_clearColor;

		Pipeline m_quadProgram;
		Buffer m_quadVBO;
		Buffer m_quadVBOIndex;
		//GLuint m_quadProjectionUniform;
		//GLuint m_quadModelUniform;
		//GLuint m_quadColorUniform;

		Pipeline m_imageProgram;
		Buffer m_imageVBO;
		Buffer m_imageVBOIndex;
		//GLuint m_imageProjectionUniform;
		//GLuint m_imageModelUniform;
		//GLuint m_imageTextureUniform;
		//GLuint m_imageColorUniform;
	};
}
