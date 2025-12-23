module;
#include "Utility.hpp"
#include "GraphicsAPI.hpp"
#include "WindowHandle.hpp"
#include <emscripten/html5.h>
#include "string.h"
#include <algorithm>
#include <limits>
#include <map>
#include <unordered_map>
module Tako.Window;

import Tako.Math;
import Tako.Bitmap;
import Tako.InputEvent;

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
			{"F1", Key::F1},
			{"F2", Key::F2},
			{"F3", Key::F3},
			{"F4", Key::F4},
			{"F5", Key::F5},
			{"F6", Key::F6},
			{"F7", Key::F7},
			{"F8", Key::F8},
			{"F9", Key::F9},
			{"F10", Key::F10},
			{"F11", Key::F11},
			{"F12", Key::F12},
			{"ArrowDown", Key::Down},
			{"ArrowLeft", Key::Left},
			{"ArrowRight", Key::Right},
			{"ArrowUp", Key::Up},
			{"Space", Key::Space},
			{"Enter", Key::Enter},
			{"Backspace", Key::Backspace},
		};

		std::pair<int, Key> GamepadMapping[]
		{
			{0, Key::Gamepad_A},
			{1, Key::Gamepad_B},
			{2, Key::Gamepad_X},
			{3, Key::Gamepad_Y},
			{4, Key::Gamepad_LB},
			{5, Key::Gamepad_RB},
			{6, Key::Gamepad_LT},
			{7, Key::Gamepad_RT},
			{8, Key::Gamepad_Select},
			{9, Key::Gamepad_Start},
			{12, Key::Gamepad_Dpad_Up},
			{13, Key::Gamepad_Dpad_Down},
			{14, Key::Gamepad_Dpad_Left},
			{15, Key::Gamepad_Dpad_Right},
		};

		std::array MouseButtonMapping
		{
			MouseButton::Left,
			MouseButton::Middle,
			MouseButton::Right,
		};
	}
	class Window::WindowImpl
	{
	public:
		WindowImpl(GraphicsAPI api)
		{
			if (api == GraphicsAPI::OpenGL)
			{
				EmscriptenWebGLContextAttributes attributes;
				emscripten_webgl_init_context_attributes(&attributes);
				attributes.alpha = false;
				attributes.antialias = false;
				m_contextHandle = emscripten_webgl_create_context(HTML_TARGET, &attributes);
				emscripten_webgl_make_context_current(m_contextHandle);
			}

			MatchCanvasToElementSize();
			ResizeCallback();
			emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, false, WindowResizeCallback);

			//emscripten_set_keypress_callback(HTML_TARGET, this, false, KeyPressCallback);
			emscripten_set_keydown_callback(HTML_TARGET, this, false, KeyPressCallback);
			emscripten_set_keyup_callback(HTML_TARGET, this, false, KeyPressCallback);
			emscripten_set_mousemove_callback(HTML_TARGET, this, false, MouseMoveCallback);
			//emscripten_set_click_callback(HTML_TARGET, this, false, MousePressCallback);
			emscripten_set_mousedown_callback(HTML_TARGET, this, false, MousePressCallback);
			emscripten_set_mouseup_callback(HTML_TARGET, this, false, MousePressCallback);
			emscripten_set_touchstart_callback(HTML_TARGET, this, false, TouchCallback);
			emscripten_set_touchend_callback(HTML_TARGET, this, false, TouchCallback);
			emscripten_set_touchmove_callback(HTML_TARGET, this, false, TouchCallback);
			emscripten_set_touchcancel_callback(HTML_TARGET, this, false, TouchCallback);
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
							m_inputCallback(evt);
						}

						{
							AxisUpdate evt;
							evt.axis = Axis::Left;
							evt.value.x = padState.axis[0];
							evt.value.y = padState.axis[1];
							m_inputCallback(evt);
							evt.axis = Axis::Right;
							evt.value.x = padState.axis[2];
							evt.value.y = padState.axis[3];
							m_inputCallback(evt);
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
		std::function<bool(InputEvent&)> m_inputCallback;
		std::unordered_map<int, EmscriptenTouchPoint> m_touches;

		void SetEventCallback(const std::function<void(Event&)>& callback)
		{
			m_callback = callback;
		}

		void SetInputCallback(const std::function<bool(InputEvent&)>& callback)
		{
			m_inputCallback = callback;
		}

		void SetFullScreenMode(Window::FullScreenMode mode)
		{
			if (mode == Window::FullScreenMode::Windowed)
			{
				emscripten_exit_fullscreen();
			}
			else
			{
				emscripten_request_fullscreen(HTML_TARGET, true);
			}
		}

		Window::FullScreenMode GetFullScreenMode()
		{
			EmscriptenFullscreenChangeEvent status;
			emscripten_get_fullscreen_status(&status);
			return status.fullscreenEnabled ? Window::FullScreenMode::FullScreen : Window::FullScreenMode::Windowed;
		}

		void Resize(int width, int height)
		{
			ResizeCanvas(width, height);
			ResizeCallback();
		}

		void ResizeCallback()
		{
			ResizeCallback(m_width, m_height);
		}

		void ResizeCallback(int width, int height)
		{
			if (m_callback)
			{
				{
					WindowResize evt;
					evt.width = width;
					evt.height = height;
					m_callback(evt);
				}

				{
					FramebufferResize evt;
					evt.width = width;
					evt.height = height;
					m_callback(evt);
				}
			}
		}

		void ResizeCanvas(int width, int height)
		{
			emscripten_set_canvas_element_size(HTML_TARGET, width, height);
			emscripten_get_canvas_element_size(HTML_TARGET, &m_width, &m_height);
		}

		void MatchCanvasToElementSize()
		{
			double width, height;
			emscripten_get_element_css_size(HTML_TARGET, &width, &height);
			ResizeCanvas(width, height);
		}

		static EM_BOOL WindowResizeCallback(int eventType, const EmscriptenUiEvent* uiEvent, void* userData)
		{
			Window::WindowImpl* win = static_cast<Window::WindowImpl*>(userData);
			win->MatchCanvasToElementSize();
			win->ResizeCallback();
			return true;
		}

		static EM_BOOL KeyPressCallback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData)
		{
			Window::WindowImpl* win = static_cast<Window::WindowImpl*>(userData);

			bool handled = false;
			if (win->m_callback)
			{
				if (KeyCodeMapping.count(keyEvent->code) > 0)
				{
					KeyPress evt;
					evt.key = KeyCodeMapping[keyEvent->code];
					switch (eventType)
					{
						case EMSCRIPTEN_EVENT_KEYDOWN:
							evt.status = KeyStatus::Down;
							break;
						case EMSCRIPTEN_EVENT_KEYUP:
							evt.status = KeyStatus::Up;
							break;
					}
					handled = win->m_inputCallback(evt);
				}

				//TODO: Checking for length == 1 might cause problems with special characters or IME
				if (eventType == EMSCRIPTEN_EVENT_KEYDOWN && strlen(keyEvent->key) == 1)
				{
					TextInputUpdate evt;
					evt.input = keyEvent->key;
					handled = win->m_inputCallback(evt);
				}
			}

			return handled;
		}

		EM_BOOL static MouseMoveCallback(int eventType, const EmscriptenMouseEvent* mouseEvent, void *userData)
		{
			Window::WindowImpl* win = static_cast<Window::WindowImpl*>(userData);
			if (win->m_callback)
			{
				MouseMove evt;
				evt.position.x = mouseEvent->clientX;
				evt.position.y = win->m_height - mouseEvent->clientY;
				return win->m_inputCallback(evt);
			}
			return false;
		}

		EM_BOOL static MousePressCallback(int eventType, const EmscriptenMouseEvent* mouseEvent, void *userData)
		{
			Window::WindowImpl* win = static_cast<Window::WindowImpl*>(userData);
			if (win->m_callback)
			{
				MouseButtonPress evt;
				evt.button = MouseButtonMapping[mouseEvent->button];
				switch (eventType)
				{
					//case EMSCRIPTEN_EVENT_CLICK:
					case EMSCRIPTEN_EVENT_MOUSEDOWN:
						evt.status = MouseButtonStatus::Down;
						break;
					case EMSCRIPTEN_EVENT_MOUSEUP:
						evt.status = MouseButtonStatus::Up;
						break;
				}
				return win->m_inputCallback(evt);
			}
			return false;
		}

		EM_BOOL static TouchCallback(int eventType, const EmscriptenTouchEvent* touchEvent, void *userData)
		{
			Window::WindowImpl* win = static_cast<Window::WindowImpl*>(userData);
			bool handled = false;

			switch (eventType)
			{
				case EMSCRIPTEN_EVENT_TOUCHSTART:
					for (int i = 0; i < touchEvent->numTouches; i++)
					{
						auto touch = touchEvent->touches[i];
						win->m_touches[touch.identifier] = touch;
					}
					break;
				case EMSCRIPTEN_EVENT_TOUCHMOVE:
					for (int i = 0; i < touchEvent->numTouches; i++)
					{
						auto touch = touchEvent->touches[i];
						if (touch.isChanged && win->m_callback)
						{
							MouseMove evt;
							evt.position.x = touch.clientX;
							evt.position.y = win->m_height - touch.clientY;
							handled = win->m_inputCallback(evt);
						}
						win->m_touches[touch.identifier] = touch;
					}
					break;
				case EMSCRIPTEN_EVENT_TOUCHEND:
				case EMSCRIPTEN_EVENT_TOUCHCANCEL:
					win->m_touches.clear();
					for (int i = 0; i < touchEvent->numTouches; i++)
					{
						auto touch = touchEvent->touches[i];
						win->m_touches[touch.identifier] = touch;
					}
			}

			return handled;
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

	Point Window::GetFramebufferSize()
	{
		return {m_impl->m_width, m_impl->m_height};
	}

	WindowHandle Window::GetHandle() const
	{
		return HTML_TARGET;
	}

	void Window::SetEventCallback(const std::function<void(Event&)>& callback)
	{
		m_impl->SetEventCallback(callback);
	}

	void Window::SetInputCallback(const std::function<bool(InputEvent&)>& callback)
	{
		m_impl->SetInputCallback(callback);
	}

	void Window::SetFullScreenMode(Window::FullScreenMode mode)
	{
		m_impl->SetFullScreenMode(mode);
	}

	Window::FullScreenMode Window::GetFullScreenMode()
	{
		return m_impl->GetFullScreenMode();
	}
}
