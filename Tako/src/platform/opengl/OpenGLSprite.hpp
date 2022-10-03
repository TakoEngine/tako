#pragma once
#include "Sprite.hpp"
#include "Texture.hpp"
#include "OpenGL.hpp"

namespace tako
{
	class OpenGLSprite : public Sprite
	{
	public:
		OpenGLSprite(const Texture texture, GLuint buffer);

		inline const Texture GetTexture() const { return m_texture; }
		inline GLuint GetBuffer() const { return m_buffer; }
		float width;
		float height;
	private:
		const Texture m_texture;
		GLuint m_buffer;
	};
}
