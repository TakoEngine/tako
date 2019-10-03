#pragma once

#include "PixelArtDrawer.hpp"
#include "OpenGLTexture.hpp"
#include <GLES2/gl2.h>

namespace tako
{
    class OpenGLPixelArtDrawer : public PixelArtDrawer
    {
    public:
        OpenGLPixelArtDrawer();
        ~OpenGLPixelArtDrawer() override;

        void Clear() override;
        void DrawRectangle(float x, float y, float w, float h, Color c) override;
        void DrawImage(float x, float y, float w, float h, const Texture* img) override;

        Texture* CreateTexture(const Bitmap& bitmap) override;

        void Resize(int w, int h);

    private:
        void SetupQuadPipeline();
        void SetupImagePipeline();

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
    };
}