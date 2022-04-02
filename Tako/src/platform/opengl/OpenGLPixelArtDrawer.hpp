#pragma once
#include "GraphicsContext.hpp"
#include "OpenGLSprite.hpp"
#include "OpenGL.hpp"

namespace tako
{
	class OpenGLPixelArtDrawer
	{
	public:
		OpenGLPixelArtDrawer(GraphicsContext* context);
		~OpenGLPixelArtDrawer();

		void SetClearColor(Color c);
		void SetTargetSize(int width, int height);
		void AutoScale();
		void SetCameraPosition(Vector2 position);
		Vector2 GetCameraPosition();
		Vector2 GetCameraViewSize();

		void Clear();
		void DrawRectangle(float x, float y, float w, float h, Color c);
		void DrawImage(float x, float y, float w, float h, const TextureHandle img, Color color = {255, 255, 255, 255});
		void DrawSprite(float x, float y, float w, float h, const Sprite* sprite, Color color = {255, 255, 255, 255});

		Texture CreateTexture(const Bitmap& bitmap);
		Sprite* CreateSprite(const Texture texture, float x, float y, float w, float h);

		void UpdateTexture(Texture& texture, const Bitmap& bitmap);

		void Resize(int w, int h);

	private:
		void DrawTextureQuad(float x, float y, float w, float h, const TextureHandle texture, GLuint buffer, Color color);
		void SetupQuadPipeline();
		void SetupImagePipeline();
		void GetDrawOffset(float& x, float& y, float& w, float& h);
		void CalculateScale();

		GraphicsContext* m_context;

		Vector2 m_cameraPosition;
		U32 m_width;
		U32 m_height;

		bool m_sizeSet;
		bool m_autoScale;
		U32 m_scale;
		U32 m_sizeW;
		U32 m_sizeH;
		Color m_clearColor;

		GLuint m_quadProgram;
		GLuint m_quadVBO;
		GLuint m_quadProjectionUniform;
		GLuint m_quadModelUniform;
		GLuint m_quadColorUniform;

		GLuint m_imageProgram;
		GLuint m_imageVBO;
		GLuint m_imageProjectionUniform;
		GLuint m_imageModelUniform;
		GLuint m_imageTextureUniform;
		GLuint m_imageColorUniform;
	};
}
