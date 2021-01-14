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
	Key ProcessKey(WPARAM wParam)
	{
		switch (wParam)
		{
		case 0x57: return Key::W;
		case 0x41: return Key::A;
		case 0x53: return Key::S;
		case 0x44: return Key::D;
		case 0x51: return Key::Q;
		case 0x45: return Key::E;
		case 0x4B: return Key::K;
		case 0x4C: return Key::L;
		case 0x58: return Key::X;
		case 0x43: return Key::C;
		case VK_UP: return Key::Up;
		case VK_DOWN: return Key::Down;
		case VK_LEFT: return Key::Left;
		case VK_RIGHT: return Key::Right;
		case VK_SPACE: return Key::Space;
		case VK_RETURN: return Key::Enter;
		//TODO: more
		default: return Key::Unknown;
		}
	}

	class Window::WindowImpl
	{
	public:
		WindowImpl(GraphicsAPI api)
		{
			WNDCLASSEX windowClass = { 0 };
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
			windowClass.hInstance = GetModuleHandle(nullptr);
			windowClass.lpfnWndProc = WindowProc;
			windowClass.lpszClassName = "tako";
			windowClass.style = CS_OWNDC;

			RegisterClassEx(&windowClass);

			RECT r = RECT();
			r.top = 0;
			r.bottom = 768;
			r.left = 0;
			r.right = 1024;

			AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, FALSE, 0);

			m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, "tako", "Tako Engine", WS_OVERLAPPEDWINDOW, 100, 100, r.right - r.left, r.bottom - r.top, NULL, NULL, GetModuleHandle(nullptr), this);
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
					LOG("WM_QUIT");
					if (m_callback)
					{
						AppQuit evt;
						m_callback(evt);
					}
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
		std::function<void(Event&)> m_callback;

		void SetEventCallback(const std::function<void(Event&)>& callback)
		{
			m_callback = callback;
		}

		static LRESULT WINAPI WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			LRESULT result = 0;
			WindowImpl* win = reinterpret_cast<WindowImpl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			switch (uMsg)
			{
			case WM_KEYDOWN:
			{
				Key key = ProcessKey(wParam);
				if (key != Key::Unknown)
				{
					KeyPress evt;
					evt.key = key;
					evt.status = KeyStatus::Down;
					win->m_callback(evt);
				}
			} break;
			case WM_KEYUP:
			{
				Key key = ProcessKey(wParam);
				if (key != Key::Unknown)
				{
					KeyPress evt;
					evt.key = key;
					evt.status = KeyStatus::Up;
					win->m_callback(evt);
				}
			} break;
			case WM_MOUSEMOVE:
			{
				POINTS pts = MAKEPOINTS(lParam);
				//LOG("{} {}", pts.x, pts.y);
				MouseMove evt;
				evt.position = Vector2(pts.x, win->m_height - pts.y);
				win->m_callback(evt);
			} break;
			case WM_SIZE:
			{
				RECT clientRect;
				GetClientRect(hwnd, &clientRect);
				win->m_width = clientRect.right - clientRect.left;
				win->m_height = clientRect.bottom - clientRect.top;
				win->Resize(win->m_width, win->m_height);

				if (win->m_callback)
				{
					WindowResize evt;
					evt.width = win->m_width;
					evt.height = win->m_height;
					win->m_callback(evt);
				}

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
				WindowClose evt;
				if (win->m_callback)
				{
					win->m_callback(evt);
				}
				/*
				if (!evt.abortQuit)
				{
					PostQuitMessage(0);
					LOG("post quit");
				}
				*/
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


	Window::Window(GraphicsAPI api) : m_impl(new WindowImpl(api))
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

	void Window::SetEventCallback(const std::function<void(Event&)>& callback)
	{
		m_impl->SetEventCallback(callback);
	}
}
