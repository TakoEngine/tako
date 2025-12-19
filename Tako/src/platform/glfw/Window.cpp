module;
#include "Utility.hpp"
#include "GraphicsAPI.hpp"
#include "WindowHandle.hpp"
//#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <map>
#include <functional>
module Tako.Window;

import Tako.Math;

namespace tako
{
	std::map<int, Key> KeyCodeMapping
	{
		{GLFW_KEY_A, Key::A},
		{GLFW_KEY_B, Key::B},
		{GLFW_KEY_C, Key::C},
		{GLFW_KEY_D, Key::D},
		{GLFW_KEY_E, Key::E},
		{GLFW_KEY_F, Key::F},
		{GLFW_KEY_G, Key::G},
		{GLFW_KEY_H, Key::H},
		{GLFW_KEY_I, Key::I},
		{GLFW_KEY_J, Key::J},
		{GLFW_KEY_K, Key::K},
		{GLFW_KEY_L, Key::L},
		{GLFW_KEY_M, Key::M},
		{GLFW_KEY_N, Key::N},
		{GLFW_KEY_O, Key::O},
		{GLFW_KEY_P, Key::P},
		{GLFW_KEY_Q, Key::Q},
		{GLFW_KEY_R, Key::R},
		{GLFW_KEY_S, Key::S},
		{GLFW_KEY_T, Key::T},
		{GLFW_KEY_U, Key::U},
		{GLFW_KEY_V, Key::V},
		{GLFW_KEY_W, Key::W},
		{GLFW_KEY_X, Key::X},
		{GLFW_KEY_Y, Key::Y},
		{GLFW_KEY_Z, Key::Z},
		{GLFW_KEY_F1, Key::F1},
		{GLFW_KEY_F2, Key::F2},
		{GLFW_KEY_F3, Key::F3},
		{GLFW_KEY_F4, Key::F4},
		{GLFW_KEY_F5, Key::F5},
		{GLFW_KEY_F6, Key::F6},
		{GLFW_KEY_F7, Key::F7},
		{GLFW_KEY_F8, Key::F8},
		{GLFW_KEY_F9, Key::F9},
		{GLFW_KEY_F10, Key::F10},
		{GLFW_KEY_F11, Key::F11},
		{GLFW_KEY_F12, Key::F12},
		{GLFW_KEY_DOWN, Key::Down},
		{GLFW_KEY_LEFT, Key::Left},
		{GLFW_KEY_RIGHT, Key::Right},
		{GLFW_KEY_UP, Key::Up},
		{GLFW_KEY_SPACE, Key::Space},
		{GLFW_KEY_ENTER, Key::Enter},
		{GLFW_KEY_BACKSPACE, Key::Backspace},
	};

	std::pair<U8, Key> GamepadMapping[]
	{
		{GLFW_GAMEPAD_BUTTON_A, Key::Gamepad_A},
		{GLFW_GAMEPAD_BUTTON_B, Key::Gamepad_B},
		{GLFW_GAMEPAD_BUTTON_X, Key::Gamepad_X},
		{GLFW_GAMEPAD_BUTTON_Y, Key::Gamepad_Y},
		{GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, Key::Gamepad_LB},
		{GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, Key::Gamepad_RB},
		//{6, Key::Gamepad_L2},
		//{7, Key::Gamepad_R2},
		{GLFW_GAMEPAD_BUTTON_BACK, Key::Gamepad_Select},
		{GLFW_GAMEPAD_BUTTON_START, Key::Gamepad_Start},
		{GLFW_GAMEPAD_BUTTON_DPAD_UP, Key::Gamepad_Dpad_Up},
		{GLFW_GAMEPAD_BUTTON_DPAD_DOWN, Key::Gamepad_Dpad_Down},
		{GLFW_GAMEPAD_BUTTON_DPAD_LEFT, Key::Gamepad_Dpad_Left},
		{GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, Key::Gamepad_Dpad_Right},
	};

	std::map<int, MouseButton> MouseCodeMapping
	{
		{GLFW_MOUSE_BUTTON_LEFT, MouseButton::Left},
		{GLFW_MOUSE_BUTTON_RIGHT, MouseButton::Right},
		{GLFW_MOUSE_BUTTON_MIDDLE, MouseButton::Middle},
	};

	void GlfwErrorCallback(int code, const char* description)
	{
		LOG_ERR("GLFW error {}: {}", code, description);
	}

	class Window::WindowImpl
	{
	public:
		WindowImpl(GraphicsAPI api)
		{
			glfwSetErrorCallback(GlfwErrorCallback);
			if (!glfwInit()) {
				LOG_ERR("Error GLFW INIT");
				return;
			}
			if (api != GraphicsAPI::OpenGL)
			{
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			}
			constexpr int initWidth = 1200;
			constexpr int initHeight = 675;
			m_window = glfwCreateWindow(initWidth, initHeight, "tako", NULL, NULL);
			m_width = initWidth;
			m_height = initHeight;
			m_fullScreenMode = FullScreenMode::Windowed;
			if (!m_window)
			{
				LOG_ERR("Error creating window");
				glfwTerminate();
				return;
			}

			glfwGetFramebufferSize(m_window, &m_frameBufferSize.x, &m_frameBufferSize.y);

			glfwSetWindowUserPointer(m_window, this);

			//LOG("GLVersion {} {}.{}", glGetString(GL_SHADING_LANGUAGE_VERSION),GLVersion.major, GLVersion.minor);

			glfwSetKeyCallback(m_window, KeyCallback);
			glfwSetCharCallback(m_window, CharCallback);
			glfwSetCursorPosCallback(m_window, CursorPositionCallback);
			glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
			glfwSetWindowSizeCallback(m_window, WindowSizeCallback);
			glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
		}

		void Poll()
		{
			glfwPollEvents();
			for (auto joyID = GLFW_JOYSTICK_1; joyID <= GLFW_JOYSTICK_LAST; joyID++)
			{
				if (glfwJoystickIsGamepad(joyID))
				{
					GLFWgamepadstate state;
					if (glfwGetGamepadState(joyID, &state))
					{
						AxisUpdate left;
						left.axis = Axis::Left;
						left.value = Vector2(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X], state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
						m_callback(left);

						AxisUpdate right;
						right.axis = Axis::Right;
						right.value = Vector2(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X], state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
						m_callback(right);

						for (auto [index, key] : GamepadMapping)
						{
							KeyPress evt;
							evt.key = key;
							evt.status = state.buttons[index] ? KeyStatus::Down : KeyStatus::Up;
							m_callback(evt);
						}

						// Emulate trigger axis as button presses
						constexpr float triggerDeadZone = 0.1f;
						KeyPress lt;
						lt.key = Key::Gamepad_LT;
						lt.status = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > triggerDeadZone ? KeyStatus::Down : KeyStatus::Up;
						m_callback(lt);

						KeyPress rt;
						rt.key = Key::Gamepad_RT;
						rt.status = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > triggerDeadZone ? KeyStatus::Down : KeyStatus::Up;
						m_callback(rt);
					}
					break; //TODO: support more than the first found controller
				}
			}
		}

		void SetEventCallback(const std::function<void(Event&)>& callback)
		{
			m_callback = callback;
		}

		void SetFullScreenMode(Window::FullScreenMode mode)
		{
			GLFWmonitor* monitor = nullptr;
			int width = m_width;
			int height = m_height;
			int refreshRate = GLFW_DONT_CARE;
			if (mode == FullScreenMode::FullScreen)
			{
				monitor = glfwGetPrimaryMonitor();
			}
			glfwSetWindowMonitor(m_window, monitor, 0, 0, width, height, refreshRate);
			m_fullScreenMode = mode;
		}

		Window::FullScreenMode GetFullScreenMode()
		{
			return m_fullScreenMode;
		}

		int m_width, m_height;
		Window::FullScreenMode m_fullScreenMode;
		Point m_frameBufferSize;
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

		static void CharCallback(GLFWwindow* window, unsigned int codepoint)
		{
			auto win = static_cast<WindowImpl*>(glfwGetWindowUserPointer(window));
			TextInputUpdate evt;
			evt.input = codepoint;
			win->m_callback(evt);
		}

		static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
		{
			auto win = static_cast<WindowImpl*>(glfwGetWindowUserPointer(window));
			MouseMove evt;
			evt.position = Vector2(xpos, win->m_height - ypos);
			win->m_callback(evt);
		}

		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
		{
			auto win = static_cast<WindowImpl*>(glfwGetWindowUserPointer(window));

			if (MouseCodeMapping.count(button) <= 0)
			{
				return;
			}

			MouseButtonPress evt;
			evt.button = MouseCodeMapping[button];
			evt.status = action == GLFW_PRESS || action == GLFW_REPEAT ? MouseButtonStatus::Down : MouseButtonStatus::Up;
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

		static void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
		{
			auto win = static_cast<WindowImpl*>(glfwGetWindowUserPointer(window));
			win->m_frameBufferSize.x = width;
			win->m_frameBufferSize.y = height;
			FramebufferResize evt;
			evt.width = width;
			evt.height = height;
			win->m_callback(evt);
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

	Point Window::GetFramebufferSize()
	{
		return m_impl->m_frameBufferSize;
	}

	WindowHandle Window::GetHandle() const
	{
		return m_impl->m_window;
	}

	void Window::SetEventCallback(const std::function<void(Event&)>& callback)
	{
		m_impl->SetEventCallback(callback);
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
