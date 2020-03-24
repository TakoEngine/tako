#include "Window.hpp"
#include "Utility.hpp"
#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include <GLFW/glfw3.h>

namespace tako
{
    class Window::WindowImpl
    {
    public:
        WindowImpl()
        {
            if (!glfwInit())
            {
                LOG_ERR("Error GLFW INIT");
                return;
            }

            //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
            m_window = glfwCreateWindow(1024, 768, "tako", NULL, NULL);
            m_width = 1024;
            m_height = 768;
            if (!m_window)
            {
                LOG_ERR("Error creating window");
                glfwTerminate();
                return;
            }

            glfwMakeContextCurrent(m_window);
            gladLoadGLES2Loader((GLADloadproc) glfwGetProcAddress);
            LOG("GLVersion {} {}.{}", glGetString(GL_SHADING_LANGUAGE_VERSION),GLVersion.major, GLVersion.minor);
        }

        void Poll()
        {
            //glClearColor(1, 0, 1, 1);
            //glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }

        GLFWwindow* m_window;
        int m_width, m_height;
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
        return glfwWindowShouldClose(m_impl->m_window);
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
        //m_impl->SetEventCallback(callback);
    }
}