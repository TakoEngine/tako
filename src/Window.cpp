#include "Window.hpp"
#include <Windows.h>
#include "Utility.hpp"
#include "Bitmap.hpp"
#include <algorithm>


using namespace tako::literals;

namespace tako
{
    class Window::WindowImpl
    {
    public:
        WindowImpl() : m_bitmap(1, 1)
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
            Resize(1024, 768);
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
        BITMAPINFO m_bitmapInfo;
        Bitmap m_bitmap;
        void* m_backBuffer;

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
                    win->Blit(win->m_bitmap);
                    win->Blit(0, 0, win->m_width, win->m_height);
                } break;
                case WM_PAINT:
                {
                    PAINTSTRUCT paint;
                    HDC deviceContext = BeginPaint(hwnd, &paint);
                    int x = paint.rcPaint.left;
                    int y = paint.rcPaint.top;
                    int width = paint.rcPaint.right - paint.rcPaint.left;
                    int height = paint.rcPaint.bottom - paint.rcPaint.top;

                    win->Blit(deviceContext, x, y, width, height);
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
            if (m_backBuffer)
            {
                VirtualFree(m_backBuffer, 0, MEM_RELEASE);
            }
            m_bitmapInfo.bmiHeader.biSize = sizeof(m_bitmapInfo.bmiHeader);
            m_bitmapInfo.bmiHeader.biWidth = width;
            m_bitmapInfo.bmiHeader.biHeight = -height;
            m_bitmapInfo.bmiHeader.biPlanes = 1;
            m_bitmapInfo.bmiHeader.biBitCount = 32;
            m_bitmapInfo.bmiHeader.biCompression = BI_RGB;

            int bitmapSize = width * height * 4;
            m_backBuffer = VirtualAlloc(NULL, bitmapSize, MEM_COMMIT, PAGE_READWRITE);
        }

        void Blit(const Bitmap& bitmap)
        {
            ASSERT(m_width >= bitmap.Width() && m_height >= bitmap.Height());
            int xscale = m_width / bitmap.Width();
            int yscale = m_height / bitmap.Height();
            int scale = xscale < yscale ? xscale : yscale;
            int xoffset = (m_width - bitmap.Width() * scale) / 2;
            int yoffset = (m_height - bitmap.Height() * scale) / 2;
            size_t bitmapPitch = bitmap.Width();
            size_t bufferPitch = m_width;

            const Color* bitmapRow = bitmap.GetData();
            Color* bufferRow = (Color*)m_backBuffer + yoffset * bufferPitch + xoffset;
            for (int y = 0; y < bitmap.Height(); y++)
            {
                const Color* bitmapPixel = bitmapRow;
                Color* bufferPixel = bufferRow;
                for (int x = 0; x < bitmap.Width(); x++)
                {
                    Color pixel = *bitmapPixel;
                    std::swap(pixel.r, pixel.b); // Swap red and blue because the windows guys wanted to look nice in the register

                    for (int j = 0; j < scale; j++)
                    {
                        for (int i = 0; i < scale; i++)
                        {
                            *(bufferPixel + i + j * bufferPitch) = pixel;
                        }
                    }

                    bitmapPixel++;
                    bufferPixel += scale;
                }

                bitmapRow += bitmapPitch;
                bufferRow += bufferPitch * scale;
            }

        }

        void Blit(int x, int y, int width, int height)
        {
            HDC deviceContext = GetDC(m_hwnd);
            Blit(deviceContext, x, y, width, height);
            ReleaseDC(m_hwnd, deviceContext);
        }

        void Blit(HDC deviceContext, int x, int y, int width, int height)
        {
            StretchDIBits(deviceContext,
                x, y, width, height, //Device Rect
                x, y, width, height, //buffer rect
                m_backBuffer,
                &m_bitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
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

    void Window::DrawBitmap(const Bitmap& bitmap)
    {
        m_impl->Blit(bitmap);
        m_impl->Blit(0, 0, GetWidth(), GetHeight());
        m_impl->m_bitmap = bitmap.Clone();
    }
}
