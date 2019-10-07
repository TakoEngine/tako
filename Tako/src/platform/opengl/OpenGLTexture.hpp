#pragma once
#include "Texture.hpp"
#include "Bitmap.hpp"
#include <GLES2/gl2.h>

namespace tako
{
    class OpenGLTexture : public Texture
    {
    public:
        explicit OpenGLTexture(const Bitmap& bitmap);
        ~OpenGLTexture() override;
        void Bind() const;

        inline int GetWidth() const { return m_width; }
        inline int GetHeight() const { return m_height; }
    private:
        GLuint m_id;
        int m_width, m_height;
    };
}
