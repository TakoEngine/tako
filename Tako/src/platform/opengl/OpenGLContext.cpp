#include "GraphicsContext.hpp"

#include "Math.hpp"
#include "Bitmap.hpp"
#include "OpenGLPixelArtDrawer.hpp"
#ifdef _WIN32
#include <Windows.h>
#include <glbinding/gl20/gl.h>
#include <glbinding/Binding.h>
using namespace gl20;

#pragma comment(lib, "opengl32.lib")
#endif
#include <GLES2/gl2.h>

namespace tako
{
	#ifdef _WIN32
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
	#endif

	static const Vector2 vertices[] =
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

	static const ImageVertex imageVertices[] =
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



	class GraphicsContext::ContextImpl
	{
	public:
		ContextImpl(WindowHandle hwnd, int width, int height)
		{
			m_handle = hwnd;
			GetPixelArtDrawer()->Resize(width, height);
			/*
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


			glDisable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			//glEnable(GL_ALPHA_TEST);
			//glAlphaFunc(GL_GREATER, 0.1);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			

			SetupQuadPipeline();
			SetupImagePipeline();
			Resize(width, height);

            Bitmap tree = Bitmap::FromFile("tree.png");
            bitmap = UploadBitmap(tree);
            */
		}
/*
		void Resize(int w, int h)
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
		*/


		void Draw()
		{
			glFlush();
			//SwapBuffers(m_hdc);
		}

		void HandleEvent(Event& evt)
		{
			switch (evt.GetType())
			{
			case tako::EventType::WindowResize:
			{
				tako::WindowResize& res = static_cast<tako::WindowResize&>(evt);
				LOG("Window Resize: {} {} {}", res.GetName(), res.width, res.height);
                GetPixelArtDrawer()->Resize(res.width, res.height);
			} break;
			}
		}

		OpenGLPixelArtDrawer* GetPixelArtDrawer()
        {
		    if (m_drawer)
            {
		        return m_drawer;
            }

		    m_drawer = new OpenGLPixelArtDrawer();
		    return m_drawer;
        }

	private:
		WindowHandle m_handle;
        OpenGLPixelArtDrawer* m_drawer = nullptr;
	};

	GraphicsContext::GraphicsContext(WindowHandle handle, int width, int height) : m_impl(new ContextImpl(handle, width, height))
	{
	}

	GraphicsContext::~GraphicsContext() = default;

	void GraphicsContext::Present()
	{
		//m_impl->Draw();
		glFlush();
	}

	void GraphicsContext::Resize(int width, int height)
	{
		m_impl->GetPixelArtDrawer()->Resize(width, height);
	}

	void GraphicsContext::HandleEvent(Event& evt)
	{
		m_impl->HandleEvent(evt);
	}

	PixelArtDrawer* GraphicsContext::CreatePixelArtDrawer()
    {
	    return m_impl->GetPixelArtDrawer();
    }
}
