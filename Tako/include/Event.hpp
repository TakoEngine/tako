#pragma once

#include <vector>
#include <functional>
#include <iostream>

namespace tako
{
	enum class EventType
	{
		WindowClose,
		WindowResize,
		KeyPress,
		AppQuit
	};

	class Event
	{
	public:
		virtual EventType GetType() const = 0;
		virtual const char* GetName() const = 0;

		friend std::ostream& operator<<(std::ostream& os, const Event& evt);
	protected:
		virtual void DebugPrint(std::ostream& os) const
		{
			os << GetName();
		}
	};



#define EVENT_CLASS_TYPE(type) \
	virtual EventType GetType() const override { return EventType::type; } \
	virtual const char* GetName() const override { return #type; }

	class WindowClose : public Event
	{
	public:
		EVENT_CLASS_TYPE(WindowClose)

		bool abortQuit = false;
	};

	class WindowResize : public Event
	{
	public:
		EVENT_CLASS_TYPE(WindowResize)

		int width;
		int height;
	protected:
		virtual void DebugPrint(std::ostream& os) const override
		{
			os << GetName() << "(" << width << "," << height << ")";
		}
	};

	enum class Key
    {
	    W = 0,
	    A,
	    S,
	    D,
	    Q,
	    E,
	    K,
	    L,
	    X,
	    C,
	    Up,
	    Down,
	    Left,
	    Right,
	    Space,
	    Enter,
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
        Gamepad_L,
        Gamepad_L2,
        Gamepad_R,
        Gamepad_R2,
        Unknown
    };

    enum class KeyStatus
    {
        Up = 0,
        Down
    };

    class KeyPress : public Event
    {
    public:
        EVENT_CLASS_TYPE(KeyPress)

        Key key;
        KeyStatus status;
    };

	class AppQuit : public Event
	{
	public:
		EVENT_CLASS_TYPE(AppQuit)
	};

	class IEventHandler
	{
	public:
		virtual void HandleEvent(Event& evt) = 0;
	};

	class CallbackEventHandler : public IEventHandler
	{
	public:
		CallbackEventHandler(std::function<void(Event&)> callback) : callback(callback)
		{}

		virtual void HandleEvent(Event& evt) override
		{
			if (callback)
			{
				callback(evt);
			}
		}
	private:
		std::function<void(Event&)> callback;
	};

	class Broadcaster
	{
	public:
		void Broadcast(Event& evt)
		{
			for (auto handler : listener)
			{
				handler->HandleEvent(evt);
			}
		}

		void Register(IEventHandler* handler)
		{
			listener.push_back(handler);
		}

	private:
		std::vector<IEventHandler*> listener;
	};
}
