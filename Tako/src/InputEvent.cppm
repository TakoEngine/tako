module;
#include <vector>
#include <functional>
#include <variant>
#include <iostream>
export module Tako.InputEvent;

import Tako.Math;

export namespace tako
{
	enum class InputEventType
	{
		KeyPress,
		TextInputUpdate,
		MouseMove,
		MouseButtonPress,
		AxisUpdate
	};

	class InputEvent
	{
	public:
		virtual InputEventType GetType() const = 0;
		virtual const char* GetName() const = 0;

		friend std::ostream& operator<<(std::ostream& os, const InputEvent& evt);
	protected:
		virtual void DebugPrint(std::ostream& os) const
		{
			os << GetName();
		}
	};

	std::ostream& operator<<(std::ostream& os, const InputEvent& evt)
	{
		evt.DebugPrint(os);
		return os;
	}


#define EVENT_CLASS_TYPE(type) \
	virtual InputEventType GetType() const override { return InputEventType::type; } \
	virtual const char* GetName() const override { return #type; }

	enum class Key
	{
		A = 0,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		Up,
		Down,
		Left,
		Right,
		Space,
		Enter,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		Backspace,
		Gamepad_A,
		Gamepad_B,
		Gamepad_X,
		Gamepad_Y,
		Gamepad_Dpad_Up,
		Gamepad_Dpad_Down,
		Gamepad_Dpad_Left,
		Gamepad_Dpad_Right,
		Gamepad_Start,
		Gamepad_Select,
		Gamepad_LB,
		Gamepad_LT,
		Gamepad_RB,
		Gamepad_RT,
		Unknown
	};

	enum class KeyStatus
	{
		Up = 0,
		Down
	};

	class KeyPress : public InputEvent
	{
	public:
		EVENT_CLASS_TYPE(KeyPress)

		Key key;
		KeyStatus status;
	};

	class TextInputUpdate : public InputEvent
	{
	public:
		EVENT_CLASS_TYPE(TextInputUpdate)

		std::variant<U32, const char*> input;
	};

	class MouseMove : public InputEvent
	{
	public:
		EVENT_CLASS_TYPE(MouseMove)

		Vector2 position;
	};

	enum class MouseButton
	{
		Left = 1,
		Right = 2,
		Middle = 3
	};

	enum class MouseButtonStatus
	{
		Up = 0,
		Down
	};

	class MouseButtonPress : public InputEvent
	{
	public:
		EVENT_CLASS_TYPE(MouseButtonPress)

		MouseButton button;
		MouseButtonStatus status;
	};

	enum class Axis
	{
		Left,
		Right,
		Unknown
	};

	class AxisUpdate : public InputEvent
	{
	public:
		EVENT_CLASS_TYPE(AxisUpdate)

		Axis axis;
		Vector2 value;
	};

	class IInputEventHandler
	{
	public:
		virtual bool HandleInputEvent(InputEvent& evt) = 0;
	};

	class InputBroadcaster
	{
	public:
		bool Broadcast(InputEvent& evt)
		{
			for (auto handler : listener)
			{
				if (handler->HandleInputEvent(evt))
				{
					return true;
				}
			}

			return false;
		}

		void Register(IInputEventHandler* handler)
		{
			//TODO: Add priority sorting
			listener.push_back(handler);
		}

	private:
		struct Listener
		{
			int priority;
			IInputEventHandler* handler;
		};
		std::vector<IInputEventHandler*> listener;
	};
}
