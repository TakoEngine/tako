#include "OpenGLTexture.hpp"

namespace tako
{

    OpenGLTexture::OpenGLTexture(GLuint id)
    {
        m_id = id;
    }

    OpenGLTexture::OpenGLTexture(const Bitmap &bitmap)
    {
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.Width(), bitmap.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetData());
    }

    OpenGLTexture::~OpenGLTexture()
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }

    void OpenGLTexture::Bind() const
    {
        glBindTexture(GL_TEXTURE_2D, m_id);
    }
}
