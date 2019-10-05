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

        void SetClearColor(Color c) override;
        void SetTargetSize(int width, int height) override;
        void AutoScale() override;
        void SetCameraPosition(Vector2 position) override;
        Vector2 GetCameraPosition() override;
        Vector2 GetCameraViewSize() override;

        void Clear() override;
        void DrawRectangle(float x, float y, float w, float h, Color c) override;
        void DrawImage(float x, float y, float w, float h, const Texture* img) override;

        Texture* CreateTexture(const Bitmap& bitmap) override;

        void Resize(int w, int h);

    private:
        void SetupQuadPipeline();
        void SetupImagePipeline();
        void GetDrawOffset(float& x, float& y, float& w, float& h);
        void CalculateScale();

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
    };
}