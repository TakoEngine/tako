#pragma once
#include "Texture.hpp"
#include "Bitmap.hpp"
#include <GLES2/gl2.h>

namespace tako
{
    class OpenGLTexture : public Texture
    {
    public:
        explicit OpenGLTexture(GLuint id);
        explicit OpenGLTexture(const Bitmap& bitmap);
        ~OpenGLTexture() override;
        void Bind() const;
    private:
        GLuint m_id;
    };
}
