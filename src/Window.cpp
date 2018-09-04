#include "Window.hpp"
#include <Windows.h>
#include "Utility.hpp"

namespace tako
{
    class Window::Impl
    {
    public:
        Impl()
        {
            WNDCLASSEX windowClass = { 0 };
            windowClass.cbSize = sizeof(WNDCLASSEX);
            windowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
            windowClass.hInstance = GetModuleHandle(nullptr);
            windowClass.lpfnWndProc = WindowProc;
            windowClass.lpszClassName = "tako";
            windowClass.style = CS_OWNDC;

            RegisterClassEx(&windowClass);

            m_hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, "tako", "Tako Engine", WS_OVERLAPPEDWINDOW, 100, 100, 1024, 768, NULL, NULL, GetModuleHandle(nullptr), this);
            SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
            ShowWindow(m_hwnd, SW_SHOWNORMAL);
        }

        void Poll()
        {
            MSG msg;
            while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    m_shouldExit = true;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        bool ShouldExit()
        {
            return m_shouldExit;
        }
    private:
        HWND m_hwnd;
        bool m_shouldExit = false;

        static LRESULT WINAPI WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            Impl* win = reinterpret_cast<Impl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (uMsg == WM_CLOSE)
            {
                LOG("Received close message");
                PostQuitMessage(0);
                return 0;
            }

            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

    };

    Window::Window() : m_impl(new Impl())
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

}
