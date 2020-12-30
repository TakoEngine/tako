#include "OpenGLContext.hpp"
#include "Math.hpp"
#include "Bitmap.hpp"
#include "OpenGLPixelArtDrawer.hpp"
#include "OpenGL.hpp"

namespace tako
{
    OpenGLContext::OpenGLContext(Window* window)
    {
        m_handle = window->GetHandle();
#ifdef TAKO_GLFW
        glfwMakeContextCurrent(m_handle);
#endif
#ifdef TAKO_GLAD
        gladLoadGLES2Loader((GLADloadproc) glfwGetProcAddress);
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
		glfwSwapBuffers(m_handle);
	}

}
