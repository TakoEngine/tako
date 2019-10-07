#pragma once
#include "Sprite.hpp"
#include "OpenGLTexture.hpp"

namespace tako
{
    class OpenGLSprite : public Sprite
    {
    public:
        OpenGLSprite(const OpenGLTexture* texture, GLuint buffer);

        inline const OpenGLTexture* GetTexture() const { return m_texture; }
        inline GLuint GetBuffer() const { return m_buffer; }
    private:
        const OpenGLTexture* m_texture;
        GLuint m_buffer;
    };
}