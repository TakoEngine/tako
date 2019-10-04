#include "Window.hpp"
#include "Bitmap.hpp"
#include <algorithm>
#include <limits>
#include <emscripten/html5.h>

using namespace tako::literals;

namespace tako
{
	class Window::WindowImpl
	{
	public:
		WindowImpl()
		{
            EmscriptenWebGLContextAttributes attributes;
            emscripten_webgl_init_context_attributes(&attributes);
            attributes.alpha = false;
            attributes.antialias = false;
            m_contextHandle = emscripten_webgl_create_context(0, &attributes);
            emscripten_webgl_make_context_current(m_contextHandle);
            double width, height;
            emscripten_get_element_css_size(0, &width, &height);
            int w, h;
            emscripten_get_canvas_element_size(0, &w, &h);
            Resize(width, height);
            emscripten_set_resize_callback(0, this, false, WindowResizeCallback);
		}


		~WindowImpl()
		{
		}

		void Poll()
		{
		}

		bool ShouldExit()
		{
			return false;
		}
		friend Window;
	private:
		int m_width, m_height;
        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE m_contextHandle;
		std::function<void(Event&)> m_callback;
		GLFWwindow* m_window;

		void SetEventCallback(const std::function<void(Event&)>& callback)
		{
			m_callback = callback;
		}

		void Resize(int width, int height)
		{
			m_width = width;
			m_height = height;
			emscripten_set_canvas_element_size(0, width, height);
		}

		static EM_BOOL WindowResizeCallback(int eventType, const EmscriptenUiEvent *uiEvent, void *userData)
        {
            Window::WindowImpl* win = static_cast<Window::WindowImpl*>(userData);
            double width, height;
            emscripten_get_element_css_size(0, &width, &height);
		    win->Resize(width, height);
            if (win->m_callback)
            {
                WindowResize evt;
                evt.width = win->m_width;
                evt.height = win->m_height;
                win->m_callback(evt);
            }

            return true;
        }
	};


	Window::Window() : m_impl(new WindowImpl())
	{
	}

	Window::~Window() = default;

	void Window::Poll()
	{
		m_impl->Poll();
	}

	bool Window::ShouldExit()
	{
		return m_impl->ShouldExit();
	}

	int Window::GetWidth()
	{
		return m_impl->m_width;
	}

	int Window::GetHeight()
	{
		return m_impl->m_height;
	}

	WindowHandle Window::GetHandle() const
	{
		return std::nullptr_t();
	}

	void Window::SetEventCallback(const std::function<void(Event&)>& callback)
	{
		m_impl->SetEventCallback(callback);
	}
}
