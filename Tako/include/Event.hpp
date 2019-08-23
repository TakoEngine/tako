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
	virtual EventType GetType() const override { return EventType::##type; } \
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
		virtual void DebugPrint(std::ostream& os) const
		{
			os << GetName() << "(" << width << "," << height << ")";
		}
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
