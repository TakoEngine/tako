#include "OpenGLContext.hpp"
#include "Math.hpp"
#include "Bitmap.hpp"
#include "OpenGLPixelArtDrawer.hpp"
#include "OpenGL.hpp"

#ifdef TAKO_WIN32
#include <Windows.h>
#pragma comment(lib, "opengl32.lib")

auto GetGLProcAddress(const char* name)
{
	auto procAddress = wglGetProcAddress(name);
	if (procAddress == nullptr)
	{
		static auto module = LoadLibrary("OPENGL32.DLL");
		procAddress = GetProcAddress(module, name);
	}

	return procAddress;
}
#endif

namespace tako
{
    OpenGLContext::OpenGLContext(Window* window)
    {
        m_handle = window->GetHandle();
#ifdef TAKO_GLFW
        glfwMakeContextCurrent(m_handle);
#ifdef TAKO_GLAD
		gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress);
#endif
#endif
#ifdef TAKO_WIN32
		auto m_hdc = GetDC(m_handle);
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
		ReleaseDC(m_handle, m_hdc);


		auto m_hrc = wglCreateContext(m_hdc);
		wglMakeCurrent(m_hdc, m_hrc);
#ifdef TAKO_GLAD
		gladLoadGLES2Loader((GLADloadproc)GetGLProcAddress);
#endif
#endif

        GetPixelArtDrawer()->Resize(window->GetWidth(), window->GetHeight());
    }

    void OpenGLContext::Resize(int w, int h)
    {
        GetPixelArtDrawer()->Resize(w, h);
    }

    void OpenGLContext::HandleEvent(Event& evt)
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

    OpenGLPixelArtDrawer* OpenGLContext::GetPixelArtDrawer()
    {
        if (m_drawer)
        {
            return m_drawer;
        }

        m_drawer = new OpenGLPixelArtDrawer();
        return m_drawer;
    }

    PixelArtDrawer* OpenGLContext::CreatePixelArtDrawer()
    {
        return GetPixelArtDrawer();
    }

	void OpenGLContext::Present()
	{
		//m_impl->Draw();
		glFlush();
#ifdef TAKO_GLFW
		glfwSwapBuffers(m_handle);
#endif
#ifdef TAKO_WIN32
		SwapBuffers(GetDC(m_handle));
#endif
	}

}
