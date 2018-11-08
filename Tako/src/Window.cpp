#include "Window.hpp"
#include <Windows.h>
#include "WinUtility.hpp"
#include "Bitmap.hpp"
#include "FileSystem.hpp"
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
            WNDCLASSEX windowClass = { 0 };
            windowClass.cbSize = sizeof(WNDCLASSEX);
            windowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
            windowClass.hInstance = GetModuleHandle(nullptr);
            windowClass.lpfnWndProc = WindowProc;
            windowClass.lpszClassName = "tako";
            windowClass.style = CS_OWNDC;

            RegisterClassEx(&windowClass);

            m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, "tako", "Tako Engine", WS_OVERLAPPEDWINDOW, 100, 100, 1024, 768, NULL, NULL, GetModuleHandle(nullptr), this);
            SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
            //SetWindowLong(m_hwnd, GWL_STYLE, 0);
            Resize(1024, 768);

            ShowWindow(m_hwnd, SW_SHOWNORMAL);
        }


        ~WindowImpl()
        { 
            //Cleanup HWND
        }

        void Poll()
        {
            MSG msg;
            while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    m_shouldExit = true;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        bool ShouldExit()
        {
            return m_shouldExit;
        }
        friend Window;
    private:
        HWND m_hwnd;
        bool m_shouldExit = false;
        int m_width, m_height;

        static LRESULT WINAPI WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            LRESULT result = 0;
            WindowImpl* win = reinterpret_cast<WindowImpl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            switch (uMsg)
            {
                case WM_SIZE:
                {
                    RECT clientRect;
                    GetClientRect(hwnd, &clientRect);
                    win->m_width = clientRect.right - clientRect.left;
                    win->m_height = clientRect.bottom - clientRect.top;
                    win->Resize(win->m_width, win->m_height);
                } break;
                case WM_GETMINMAXINFO:
                {
                    if (!win)
                    {
                        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
                        break;
                    }

                    RECT clientRect, windowRect;
                    GetClientRect(hwnd, &clientRect);
                    LONG clientWidth = clientRect.right - clientRect.left;
                    LONG clientHeight = clientRect.bottom - clientRect.top;
                    GetWindowRect(hwnd, &windowRect);
                    LONG windowWidth = windowRect.right - windowRect.left;
                    LONG windowHeight = windowRect.bottom - windowRect.top;
                    
                    /*
                    MINMAXINFO* mmi = (MINMAXINFO*)lParam;
                    mmi->ptMinTrackSize.x = win->m_bitmap.Width() + windowWidth - clientWidth;
                    mmi->ptMinTrackSize.y = win->m_bitmap.Height() + windowHeight - clientHeight;
                    */
                } break;
                case WM_PAINT:
                {
                    PAINTSTRUCT paint;
                    HDC deviceContext = BeginPaint(hwnd, &paint);
                    int x = paint.rcPaint.left;
                    int y = paint.rcPaint.top;
                    int width = paint.rcPaint.right - paint.rcPaint.left;
                    int height = paint.rcPaint.bottom - paint.rcPaint.top;

                    EndPaint(hwnd, &paint);
                } break;
                case WM_CLOSE:
                {
                    LOG("Received close message");
                    PostQuitMessage(0);
                } break;
                default: result = DefWindowProc(hwnd, uMsg, wParam, lParam);
            }

            return result;
        }

        void Resize(int width, int height)
        {
            m_width = width;
            m_height = height;
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
        return m_impl->m_hwnd;
    }
}