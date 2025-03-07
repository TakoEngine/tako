module;
#include "Utility.hpp"
#include <array>
#include <cstddef>
export module Tako.Input;

import Tako.Math;
import Tako.Event;

namespace tako
{
	export class Input : public IEventHandler
	{
	public:
		virtual void HandleEvent(Event& evt) override
		{
			switch (evt.GetType())
			{
				case tako::EventType::KeyPress:
				{
					tako::KeyPress& press = static_cast<tako::KeyPress &>(evt);
					if (press.key != Key::Unknown)
					{
						auto k = static_cast<size_t>(press.key);
						m_activeChangedKeys[k] = m_activeChangedKeys[k] || activeKeys[k] != press.status;
						activeKeys[k] = press.status;
					}
				} break;
				case tako::EventType::MouseMove:
				{
					tako::MouseMove& move = static_cast<tako::MouseMove&>(evt);
					m_curMousePosition = move.position;
				} break;
				case tako::EventType::AxisUpdate:
				{
					tako::AxisUpdate& update = static_cast<tako::AxisUpdate&>(evt);
					m_axis[static_cast<size_t>(update.axis)] = update.value;
				} break;
			}
		}


		void Update();
		bool GetKey(Key key);
		bool GetKeyDown(Key key);
		bool GetKeyUp(Key key);
		bool GetAnyDown();
		Vector2 GetAxis(Axis axis);
		Vector2 GetMousePosition();
		Vector2 GetMouseMovement();
	private:
		Vector2 m_mousePosition;
		Vector2 m_prevMousePosition;
		Vector2 m_curMousePosition;
		std::array<KeyStatus, static_cast<size_t>(Key::Unknown)> activeKeys = {KeyStatus::Up};
		std::array<KeyStatus, static_cast<size_t>(Key::Unknown)> keys = {KeyStatus::Up};
		std::array<KeyStatus, static_cast<size_t>(Key::Unknown)> prevKeys = {KeyStatus::Up};
		std::array<bool, static_cast<size_t>(Key::Unknown)> m_activeChangedKeys = { false };
		std::array<bool, static_cast<size_t>(Key::Unknown)> m_changedKeys = { false };
		std::array<Vector2, static_cast<size_t>(Axis::Unknown)> m_axis = {Vector2()};
	};
}



namespace tako
{
	bool Input::GetKey(Key key)
	{
		return keys[static_cast<size_t>(key)] == KeyStatus::Down;
	}

	bool Input::GetKeyDown(Key key)
	{
		return m_changedKeys[static_cast<size_t>(key)] && prevKeys[static_cast<size_t>(key)] == KeyStatus::Up;
	}

	bool Input::GetKeyUp(Key key)
	{
		return m_changedKeys[static_cast<size_t>(key)] && prevKeys[static_cast<size_t>(key)] == KeyStatus::Down;
	}

	bool Input::GetAnyDown()
	{
		for (int i = 0; i < (int) tako::Key::Unknown; i++)
		{
			if (GetKeyDown((tako::Key) i))
			{
				return true;
			}
		}

		return false;
	}

	Vector2 Input::GetAxis(Axis axis)
	{
		return m_axis[static_cast<size_t>(axis)];
	}

	Vector2 Input::GetMousePosition()
	{
		return m_mousePosition;
	}

	Vector2 Input::GetMouseMovement()
	{
		return m_mousePosition - m_prevMousePosition;
	}

	void Input::Update()
	{
		prevKeys = keys;
		keys = activeKeys;
		m_changedKeys = m_activeChangedKeys;
		m_activeChangedKeys = { false };
		m_prevMousePosition = m_mousePosition;
		m_mousePosition = m_curMousePosition;
	}
}

