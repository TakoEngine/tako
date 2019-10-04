#include "OpenGLPixelArtDrawer.hpp"
#include <vector>

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

        constexpr ImageVertex imageVertices[] =
        {
            { {0, 0}, {0, 0}},
            { {1, 0}, {1, 0}},
            { {1, 1}, {1, 1}},
            { {0, 0}, {0, 0}},
            { {1, 1}, {1, 1}},
            { {0, 1}, {0, 1}}
        };

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

    void OpenGLPixelArtDrawer::Clear()
    {
        glClearColor(0, 0.5f, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLPixelArtDrawer::DrawRectangle(float x, float y, float w, float h, Color c)
    {
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
        glUseProgram(m_imageProgram);

        Matrix4 mat = Matrix4::identity;
        mat.translate(x, y, 0);
        mat.scale(w, h, 1);
        glUniformMatrix4fv(m_imageModelUniform, 1, GL_FALSE, &mat[0]);

        //glActiveTexture(GL_TEXTURE0);
        dynamic_cast<const OpenGLTexture*>(img)->Bind();
        //glUniform1i(m_imageTextureUniform, 0);

        glBindBuffer(GL_ARRAY_BUFFER, m_imageVBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImageVertex), NULL);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImageVertex), (void*)offsetof(ImageVertex, texcoord));
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void OpenGLPixelArtDrawer::Resize(int w, int h)
    {
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(imageVertices), imageVertices, GL_STATIC_DRAW);

        m_imageProjectionUniform = glGetUniformLocation(m_imageProgram, "projection");
        m_imageModelUniform = glGetUniformLocation(m_imageProgram, "model");
        m_imageTextureUniform = glGetUniformLocation(m_imageProgram, "texture");

        auto err = glGetError();
        if (err != GL_NO_ERROR)
        {
            LOG("error pip!");
        }
    }


}