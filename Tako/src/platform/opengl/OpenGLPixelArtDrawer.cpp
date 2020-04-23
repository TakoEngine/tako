#include "OpenGLPixelArtDrawer.hpp"
#include <vector>
#include <array>

namespace tako
{
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

        struct ImageVertex
        {
            Vector2 position;
            Vector2 texcoord;
        };

        constexpr std::array<ImageVertex, 6> CreateImageVertices(float x, float y, float w, float h)
        {
            return
            {{
                    { {0, 0}, {x    , y    } },
                    { {1, 0}, {x + w, y    } },
                    { {1, 1}, {x + w, y + h } },
                    { {0, 0}, {x    , y    }} ,
                    { {1, 1}, {x + w, y + h} },
                    { {0, 1}, {x    , y + h} }
            }};
        }

        constexpr std::array<ImageVertex, 6> imageVertices = CreateImageVertices(0, 0, 1, 1);

        constexpr const char* quadVertexShader =
            #include "quad.vert.glsl"
        ;

        constexpr const char* quadFragmentShader =
            #include "quad.frag.glsl"
        ;

        constexpr const char* imageVertexShader =
            #include "image.vert.glsl"
        ;

        constexpr const char* imageFragmentShader =
            #include "image.frag.glsl"
        ;
    }


    OpenGLPixelArtDrawer::OpenGLPixelArtDrawer()
    {
        m_scale = 1;
        m_sizeSet = false;
        glClearColor(0, 0, 0, 1);
        glDisable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        SetupQuadPipeline();
        SetupImagePipeline();
    }

    OpenGLPixelArtDrawer::~OpenGLPixelArtDrawer()
    {

    }

    void OpenGLPixelArtDrawer::SetClearColor(Color c)
    {
        m_clearColor = c;
        if (!m_sizeSet)
        {
            glClearColor(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
        }
    }

    void OpenGLPixelArtDrawer::SetTargetSize(int width, int height)
    {
        m_sizeW = width;
        m_sizeH = height;

        if (m_sizeSet || m_autoScale)
        {
            CalculateScale();
        }
        if (m_sizeSet)
        {
            glClearColor(0, 0, 0, 1);
        }
    }

    void OpenGLPixelArtDrawer::AutoScale()
    {
        m_autoScale = true;
        CalculateScale();
    }

    void OpenGLPixelArtDrawer::SetCameraPosition(Vector2 position)
    {
        m_cameraPosition = position;
    }

    Vector2 OpenGLPixelArtDrawer::GetCameraPosition()
    {
        return m_cameraPosition;
    }

    Vector2 OpenGLPixelArtDrawer::GetCameraViewSize()
    {
        return
        {
            static_cast<float>(m_width) / m_scale,
            static_cast<float>(m_height) / m_scale
        };
    }

    void OpenGLPixelArtDrawer::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (m_sizeSet)
        {
            DrawRectangle(0, 0, m_sizeW, m_sizeH, m_clearColor);
        }
    }

    void OpenGLPixelArtDrawer::DrawRectangle(float x, float y, float w, float h, Color c)
    {
        GetDrawOffset(x, y, w, h);
        glUseProgram(m_quadProgram);

        Matrix4 mat = Matrix4::identity;
        mat.translate(x, y, 0);
        mat.scale(w, h, 1);
        glUniformMatrix4fv(m_quadModelUniform, 1, GL_FALSE, &mat[0]);

        float col[4] = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
        glUniform4fv(m_quadColorUniform, 1, col);


        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void OpenGLPixelArtDrawer::DrawImage(float x, float y, float w, float h, const Texture* img)
    {
        DrawTextureQuad(x, y, w, h, dynamic_cast<const OpenGLTexture*>(img), m_imageVBO);
    }

    void OpenGLPixelArtDrawer::DrawSprite(float x, float y, float w, float h, const Sprite* sprite)
    {
        auto s = dynamic_cast<const OpenGLSprite*>(sprite);
        DrawTextureQuad(x, y, w, h, s->GetTexture(), s->GetBuffer());
    }

    void OpenGLPixelArtDrawer::Resize(int w, int h)
    {
        m_width = w;
        m_height = h;
        if (m_sizeSet || m_autoScale)
        {
            CalculateScale();
        }
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
    }

    Texture* OpenGLPixelArtDrawer::CreateTexture(const Bitmap &bitmap)
    {
        return new OpenGLTexture(bitmap);
    }

    Sprite* OpenGLPixelArtDrawer::CreateSprite(const Texture* texture, float x, float y, float w, float h)
    {
        auto tex = dynamic_cast<const OpenGLTexture*>(texture);
        float sX = x / tex->GetWidth();
        float sY = y / tex->GetHeight();
        float sW = w / tex->GetWidth();
        float sH = h / tex->GetHeight();
        auto vertices = CreateImageVertices(sX, sY, sW, sH);

        GLuint spriteVBO = 0;
        glGenBuffers(1, &spriteVBO);
        glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ImageVertex), &vertices[0], GL_STATIC_DRAW);

        return new OpenGLSprite(tex, spriteVBO);
    }

    void OpenGLPixelArtDrawer::UpdateTexture(Texture* texture, const Bitmap& bitmap)
    {
        auto tex = dynamic_cast<OpenGLTexture*>(texture);
        tex->Update(bitmap);
    }

    namespace
    {
        static GLuint CompileShader(const char* shaderSource, GLenum type)
        {
            const GLchar* shaderPointer = shaderSource;
            GLuint sh = glCreateShader(type);
            glShaderSource(sh, 1, &shaderPointer, NULL);
            glCompileShader(sh);

            GLint isCompiled = 0;
            glGetShaderiv(sh, GL_COMPILE_STATUS, &isCompiled);
            if(isCompiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &maxLength);

                // The maxLength includes the NULL character
                std::vector<GLchar> errorLog(maxLength);
                glGetShaderInfoLog(sh, maxLength, &maxLength, &errorLog[0]);
                LOG_ERR("{}", std::string(errorLog.begin(),errorLog.end()));

                // Provide the infolog in whatever manor you deem best.
                // Exit with failure.
                glDeleteShader(sh); // Don't leak the shader.
                return 0;
            }

            auto err = glGetError();
            if (err != GL_NO_ERROR)
            {
                LOG("error compiling shader!");
            }

            return sh;
        }
    }

    void OpenGLPixelArtDrawer::DrawTextureQuad(float x, float y, float w, float h, const OpenGLTexture* texture, GLuint buffer)
    {
        GetDrawOffset(x, y, w, h);
        glUseProgram(m_imageProgram);

        Matrix4 mat = Matrix4::identity;
        mat.translate(x, y, 0);
        mat.scale(w, h, 1);
        glUniformMatrix4fv(m_imageModelUniform, 1, GL_FALSE, &mat[0]);

        //glActiveTexture(GL_TEXTURE0);
        texture->Bind();
        //glUniform1i(m_imageTextureUniform, 0);
        static auto posLocation = glGetAttribLocation(m_imageProgram, "position");
        static auto texLocation = glGetAttribLocation(m_imageProgram, "texcoord");

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(posLocation, 2, GL_FLOAT, GL_FALSE, sizeof(ImageVertex), (void*)offsetof(ImageVertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(texLocation, 2, GL_FLOAT, GL_FALSE, sizeof(ImageVertex), (void*)offsetof(ImageVertex, texcoord));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void OpenGLPixelArtDrawer::SetupQuadPipeline()
    {
        m_quadProgram = glCreateProgram();
        glAttachShader(m_quadProgram, CompileShader(quadVertexShader, GL_VERTEX_SHADER));
        glAttachShader(m_quadProgram, CompileShader(quadFragmentShader, GL_FRAGMENT_SHADER));
        glLinkProgram(m_quadProgram);
        auto err = glGetError();
        if (err != GL_NO_ERROR)
        {
            LOG("error shader");
        }

        m_quadVBO = 0;
        glGenBuffers(1, &m_quadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        m_quadProjectionUniform = glGetUniformLocation(m_quadProgram, "projection");
        m_quadModelUniform = glGetUniformLocation(m_quadProgram, "model");
        m_quadColorUniform = glGetUniformLocation(m_quadProgram, "color");
    }

    void OpenGLPixelArtDrawer::SetupImagePipeline()
    {
        m_imageProgram = glCreateProgram();
        glAttachShader(m_imageProgram, CompileShader(imageVertexShader, GL_VERTEX_SHADER));
        glAttachShader(m_imageProgram, CompileShader(imageFragmentShader, GL_FRAGMENT_SHADER));
        glLinkProgram(m_imageProgram);

        m_imageVBO = 0;
        glGenBuffers(1, &m_imageVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_imageVBO);
        glBufferData(GL_ARRAY_BUFFER, imageVertices.size() * sizeof(ImageVertex), &imageVertices[0], GL_STATIC_DRAW);

        m_imageProjectionUniform = glGetUniformLocation(m_imageProgram, "projection");
        m_imageModelUniform = glGetUniformLocation(m_imageProgram, "model");
        m_imageTextureUniform = glGetUniformLocation(m_imageProgram, "texture");

        auto err = glGetError();
        if (err != GL_NO_ERROR)
        {
            LOG("error pip!");
        }
    }

    void OpenGLPixelArtDrawer::GetDrawOffset(float& x, float& y, float& w, float& h)
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

    void OpenGLPixelArtDrawer::CalculateScale()
    {
        int wScale = m_width / m_sizeW;
        int hScale = m_height / m_sizeH;
        m_scale = std::max(1, std::min(wScale, hScale));
        LOG("SCALE {}", m_scale);
    }


}