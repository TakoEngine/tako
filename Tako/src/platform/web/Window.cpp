#include "Window.hpp"
#include "Bitmap.hpp"
#include "FileSystem.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <vector>
#include <array>
#include <set>
#include <limits>

using namespace tako::literals;

namespace tako
{
	class Window::WindowImpl
	{
	public:
		WindowImpl()
		{
			if (!glfwInit())
			{
				LOG_ERR("Error initializing GLFW");
				return;
			}

			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
			m_window = glfwCreateWindow(1024, 768, "Hello Web", NULL, NULL);
			if (!m_window)
			{
				LOG_ERR("Error creating Window");
				return;
			}

			glfwSetWindowUserPointer(m_window, this);

			glfwSetFramebufferSizeCallback(m_window, FrameBufferSizeCallback);
			glfwMakeContextCurrent(m_window);
		}


		~WindowImpl()
		{
			glfwTerminate();
		}

		void Poll()
		{
			//glClearColor(0, 0.5f, 0, 1);
			//glClear(GL_COLOR_BUFFER_BIT);
			glfwSwapBuffers(m_window);
			glfwPollEvents();
		}

		bool ShouldExit()
		{
			return glfwWindowShouldClose(m_window);
		}
		friend Window;
	private:
		int m_width, m_height;
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
		}

		static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
        {
            Window::WindowImpl* win = static_cast<Window::WindowImpl*>(glfwGetWindowUserPointer(window));
		    win->Resize(width, height);
            if (win->m_callback)
            {
                WindowResize evt;
                evt.width = win->m_width;
                evt.height = win->m_height;
                win->m_callback(evt);
            }
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
