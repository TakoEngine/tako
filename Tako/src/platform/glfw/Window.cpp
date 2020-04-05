#include "Window.hpp"
#include "Utility.hpp"
#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <map>

namespace tako
{
    namespace
    {
        std::map<int, Key> KeyCodeMapping
        {
                {GLFW_KEY_W, Key::W},
                {GLFW_KEY_A, Key::A},
                {GLFW_KEY_S, Key::S},
                {GLFW_KEY_D, Key::D},
                {GLFW_KEY_Q, Key::Q},
                {GLFW_KEY_E, Key::E},
                {GLFW_KEY_DOWN, Key::Down},
                {GLFW_KEY_LEFT, Key::Left},
                {GLFW_KEY_RIGHT, Key::Right},
                {GLFW_KEY_UP, Key::Up},
                {GLFW_KEY_SPACE, Key::Space},
                {GLFW_KEY_ENTER, Key::Enter}
        };
    }

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

            glfwSetWindowUserPointer(m_window, this);

            glfwMakeContextCurrent(m_window);
            gladLoadGLES2Loader((GLADloadproc) glfwGetProcAddress);
            LOG("GLVersion {} {}.{}", glGetString(GL_SHADING_LANGUAGE_VERSION),GLVersion.major, GLVersion.minor);

            glfwSetKeyCallback(m_window, KeyCallback);
            glfwSetWindowSizeCallback(m_window, WindowSizeCallback);
        }

        void Poll()
        {
            //glClearColor(1, 0, 1, 1);
            //glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }

        void SetEventCallback(const std::function<void(Event&)>& callback)
        {
            m_callback = callback;
        }

        int m_width, m_height;
        GLFWwindow* m_window;
        std::function<void(Event&)> m_callback;

        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            auto win = static_cast<WindowImpl*>(glfwGetWindowUserPointer(window));

            if (KeyCodeMapping.count(key) <= 0)
            {
                return;
            }

            KeyPress evt;
            evt.key = KeyCodeMapping[key];
            if (action == GLFW_PRESS || action == GLFW_REPEAT)
            {
                evt.status = KeyStatus::Down;
            }
            else if (action == GLFW_RELEASE)
            {
                evt.status = KeyStatus::Up;
            }

            win->m_callback(evt);
        }

        static void WindowSizeCallback(GLFWwindow* window, int width, int height)
        {
            auto win = static_cast<WindowImpl*>(glfwGetWindowUserPointer(window));
            win->m_width = width;
            win->m_height = height;
            WindowResize evt;
            evt.width = width;
            evt.height = height;
            win->m_callback(evt);
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
        m_impl->SetEventCallback(callback);
    }
}