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

		//GetPixelArtDrawer()->Resize(window->GetWidth(), window->GetHeight());
	}

	void OpenGLContext::Resize(int w, int h)
	{
		//GetPixelArtDrawer()->Resize(w, h);
	}

	void OpenGLContext::HandleEvent(Event& evt)
	{
		switch (evt.GetType())
		{
		case tako::EventType::WindowResize:
		{
			tako::WindowResize& res = static_cast<tako::WindowResize&>(evt);
			LOG("Window Resize: {} {} {}", res.GetName(), res.width, res.height);
			//GetPixelArtDrawer()->Resize(res.width, res.height);
		} break;
		}
	}

	Texture OpenGLContext::CreateTexture(const Bitmap& bitmap)
	{
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.Width(), bitmap.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetData());

		Texture t;
		t.width = bitmap.Width();
		t.height = bitmap.Height();
		t.handle.value = tex;
		return t;
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
