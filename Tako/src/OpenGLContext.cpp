#include "GraphicsContext.hpp"
#include <Windows.h>
#include <glbinding/gl20/gl.h>
#include <glbinding/Binding.h>
#include "Math.hpp"
#include "Bitmap.hpp"
using namespace gl20;

#pragma comment(lib, "opengl32.lib")

namespace tako
{
    glbinding::ProcAddress GetGLProcAddress(const char* name)
    {
        auto procAddress = reinterpret_cast<glbinding::ProcAddress>(wglGetProcAddress(name));
        if (procAddress == nullptr)
        {
            static auto module = LoadLibrary("OPENGL32.DLL");
            procAddress = reinterpret_cast<glbinding::ProcAddress>(GetProcAddress(module, name));
        }

        return procAddress;
    }

    static const Vector2 vertices[] =
    {
        { 0, 0},
        { 1, 0},
        { 1, 1},
        { 0, 0},
        { 1, 1},
        { 0, 1}
    };

    constexpr const char* quadVertexShader = R"GLSL(
        #version 110

        attribute vec2 position;

        uniform mat4 projection;
        uniform mat4 model;

        void main()
        {
            gl_Position = projection * model * vec4(position, 0.0, 1.0);
        }
    )GLSL";

    constexpr const char* quadFragmentShader = R"GLSL(
        #version 110

		uniform vec4 color;

        void main()
        {
            gl_FragColor = color;
        }
    )GLSL";

    class GraphicsContext::ContextImpl
    {
    public:
        ContextImpl(HWND hwnd)
        {
            m_hdc = GetDC(hwnd);
            PIXELFORMATDESCRIPTOR pfd;

            memset(&pfd, 0, sizeof(pfd));

            pfd.nSize = sizeof(pfd);

            pfd.nVersion = 1;

            pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;

            pfd.iPixelType = PFD_TYPE_RGBA;

            pfd.cColorBits = 32;



            int pf = ChoosePixelFormat(m_hdc, &pfd);

            SetPixelFormat(m_hdc, pf, &pfd);

            DescribePixelFormat(m_hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

            ReleaseDC(hwnd, m_hdc);



            m_hrc = wglCreateContext(m_hdc);

            wglMakeCurrent(m_hdc, m_hrc);

            
            glbinding::Binding::initialize(GetGLProcAddress);
            glClearColor(0, 0.5f, 0, 1);   

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0.1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			SetupQuadPipeline();
            Resize(1024, 768);
            
            //Bitmap map(64, 64);
            //map.Clear({ 255, 128, 128, 255 });

            //map.FillRect(32, 0, 32, 32, {255, 0, 0, 255});
            //Bitmap map = Bitmap::FromFile("adasd");
            //bitmap = UploadBitmap(map);
        }

        void Resize(int w, int h)
        {
            glViewport(0, 0, w, h);

			Matrix4 ortho = Matrix4::transpose(Matrix4::ortho(0, w, h, 0, 0, 100));
			glUseProgram(m_quadProgram);
			glUniformMatrix4fv(m_quadProjectionUniform, 1, GL_FALSE, &ortho[0]);
        }

        void DrawSquare(float x, float y, float w, float h, Color c)
        {
			glUseProgram(m_quadProgram);

			Matrix4 mat = Matrix4::identity;
			mat.translate(x, y, 0);
			mat.scale(w, h, 1);
			glUniformMatrix4fv(m_quadModelUniform, 1, GL_FALSE, &mat[0]);

			float col[4] = {c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f};
			glUniform4fv(m_quadColorUniform, 1, col);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
            glVertexAttribPointer(0, 2, GLenum::GL_FLOAT, GL_FALSE, 0, NULL);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        static GLuint UploadBitmap(const Bitmap& bitmap)
        {
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.Width(), bitmap.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetData());

            return texture;
        }


        static GLuint CompileShader(const char* shaderSource, GLenum type)
        {
            const GLchar* shaderPointer = shaderSource;
            GLuint sh = glCreateShader(type);
            glShaderSource(sh, 1, &shaderPointer, NULL);
            glCompileShader(sh);

            return sh;
        }

        void SetupQuadPipeline()
        {
            m_quadProgram = glCreateProgram();
            glAttachShader(m_quadProgram, CompileShader(quadVertexShader, GL_VERTEX_SHADER));
            glAttachShader(m_quadProgram, CompileShader(quadFragmentShader, GL_FRAGMENT_SHADER));
            glLinkProgram(m_quadProgram);

            m_quadVBO = 0;
            glGenBuffers(1, &m_quadVBO);
            glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            m_quadProjectionUniform = glGetUniformLocation(m_quadProgram, "projection");
            m_quadModelUniform = glGetUniformLocation(m_quadProgram, "model");
			m_quadColorUniform = glGetUniformLocation(m_quadProgram, "color");
        }

        void Draw()
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            DrawSquare(100, 100, 100, 100, Color("#FFFF00AA"));
			DrawSquare(0, 0, 100, 100, Color("#00FFFF"));

            glFlush();
            SwapBuffers(m_hdc);
        }
    private:
        HDC m_hdc;
        HGLRC m_hrc;
        GLuint bitmap;

        GLuint m_quadProgram;
        GLuint m_quadVBO;
        GLuint m_quadProjectionUniform;
        GLuint m_quadModelUniform;
		GLuint m_quadColorUniform;
    };

    GraphicsContext::GraphicsContext(Window& window) : m_impl(new ContextImpl(window.GetHandle()))
    {
    }

    GraphicsContext::~GraphicsContext() = default;

    void GraphicsContext::Present()
    {
        m_impl->Draw();
    }
}