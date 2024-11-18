#include "Window.hpp"
#include "string.h"
#include <algorithm>
#include <limits>
#include <map>
#include <emscripten/html5.h>

import Tako.Math;
import Tako.Bitmap;

using namespace tako::literals;

const char* HTML_TARGET = "#canvas";

namespace tako
{
	namespace
	{
		struct StrCmp
		{
			bool operator()(const EM_UTF8* a, const EM_UTF8* b) const
			{
				return std::strcmp(a, b) < 0;
			}
		};

		std::map<const EM_UTF8*, Key, StrCmp> KeyCodeMapping
		{
			{"KeyA", Key::A},
			{"KeyB", Key::B},
			{"KeyC", Key::C},
			{"KeyD", Key::D},
			{"KeyE", Key::E},
			{"KeyF", Key::F},
			{"KeyG", Key::G},
			{"KeyH", Key::H},
			{"KeyI", Key::I},
			{"KeyJ", Key::J},
			{"KeyK", Key::K},
			{"KeyL", Key::L},
			{"KeyM", Key::M},
			{"KeyN", Key::N},
			{"KeyO", Key::O},
			{"KeyP", Key::P},
			{"KeyQ", Key::Q},
			{"KeyR", Key::R},
			{"KeyS", Key::S},
			{"KeyT", Key::T},
			{"KeyU", Key::U},
			{"KeyV", Key::V},
			{"KeyW", Key::W},
			{"KeyX", Key::X},
			{"KeyY", Key::Y},
			{"KeyZ", Key::Z},
			{"ArrowDown", Key::Down},
			{"ArrowLeft", Key::Left},
			{"ArrowRight", Key::Right},
			{"ArrowUp", Key::Up},
			{"Space", Key::Space},
			{"Enter", Key::Enter}
		};

		std::pair<int, Key> GamepadMapping[]
		{
			{0, Key::Gamepad_A},
			{1, Key::Gamepad_B},
			{2, Key::Gamepad_X},
			{3, Key::Gamepad_Y},
			{4, Key::Gamepad_L},
			{5, Key::Gamepad_R},
			{6, Key::Gamepad_L2},
			{7, Key::Gamepad_R2},
			{8, Key::Gamepad_Select},
			{9, Key::Gamepad_Start},
			{12, Key::Gamepad_Dpad_Up},
			{13, Key::Gamepad_Dpad_Down},
			{14, Key::Gamepad_Dpad_Left},
			{15, Key::Gamepad_Dpad_Right},
		};
	}
	class Window::WindowImpl
	{
	public:
		WindowImpl()
		{
			EmscriptenWebGLContextAttributes attributes;
			emscripten_webgl_init_context_attributes(&attributes);
			attributes.alpha = false;
			attributes.antialias = false;
			m_contextHandle = emscripten_webgl_create_context(HTML_TARGET, &attributes);
			emscripten_webgl_make_context_current(m_contextHandle);
			double width, height;
			emscripten_get_element_css_size(HTML_TARGET, &width, &height);
			Resize(width, height);
			emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, false, WindowResizeCallback);

			emscripten_set_keypress_callback(HTML_TARGET, this, false, KeyPressCallback);
			emscripten_set_keydown_callback(HTML_TARGET, this, false, KeyPressCallback);
			emscripten_set_keyup_callback(HTML_TARGET, this, false, KeyPressCallback);
			emscripten_set_mousemove_callback(HTML_TARGET, this, false, MouseMoveCallback);
		}


		~WindowImpl()
		{
		}

		void Poll()
		{
			auto status = emscripten_sample_gamepad_data();
			if (status == EMSCRIPTEN_RESULT_SUCCESS)
			{
				auto numGamepads = emscripten_get_num_gamepads();
				EmscriptenGamepadEvent padState;
				for (int i = 0; i < numGamepads; i++)
				{
					status = emscripten_get_gamepad_status(i, &padState);
					if (status == EMSCRIPTEN_RESULT_SUCCESS && padState.connected)
					{
						for (auto [index, key] : GamepadMapping)
						{
							if (index >= padState.numButtons)
							{
								continue;
							}
							KeyPress evt;
							evt.key = key;
							evt.status = padState.digitalButton[index] ? KeyStatus::Down : KeyStatus::Up;
							m_callback(evt);
						}

						{
							AxisUpdate evt;
							evt.axis = Axis::Left;
							evt.value.x = padState.axis[0];
							evt.value.y = -padState.axis[1];
							m_callback(evt);
							evt.axis = Axis::Right;
							evt.value.x = padState.axis[2];
							evt.value.y = -padState.axis[3];
							m_callback(evt);
						}

						break;
					}
				}
			}
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
			emscripten_set_canvas_element_size(HTML_TARGET, width, height);
			if (m_callback)
			{
				WindowResize evt;
				evt.width = width;
				evt.height = height;
				m_callback(evt);
			}
		}

		static EM_BOOL WindowResizeCallback(int eventType, const EmscriptenUiEvent* uiEvent, void* userData)
		{
			Window::WindowImpl* win = static_cast<Window::WindowImpl*>(userData);
			double width, height;
			emscripten_get_element_css_size(HTML_TARGET, &width, &height);
			win->Resize(width, height);
			return true;
		}

		static EM_BOOL KeyPressCallback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData)
		{
			Window::WindowImpl* win = static_cast<Window::WindowImpl*>(userData);
			if (KeyCodeMapping.count(keyEvent->code) <= 0)
			{
				return false;
			}
			if (win->m_callback)
			{
				KeyPress evt;
				evt.key = KeyCodeMapping[keyEvent->code];
				switch (eventType)
				{
					case EMSCRIPTEN_EVENT_KEYPRESS:
					case EMSCRIPTEN_EVENT_KEYDOWN:
						evt.status = KeyStatus::Down;
						break;
					case EMSCRIPTEN_EVENT_KEYUP:
						evt.status = KeyStatus::Up;
						break;
				}
				win->m_callback(evt);
			}

			return true;
		}

		EM_BOOL static MouseMoveCallback(int eventType, const EmscriptenMouseEvent* mouseEvent, void *userData)
		{
			Window::WindowImpl* win = static_cast<Window::WindowImpl*>(userData);
			if (win->m_callback)
			{
				MouseMove evt;
				evt.position.x = mouseEvent->clientX;
				evt.position.y = win->m_height - mouseEvent->clientY;
				win->m_callback(evt);
				return true;
			}
			return false;
		}
	};


	Window::Window(GraphicsAPI api) : m_impl(new WindowImpl())
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
