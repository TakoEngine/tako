#include "Input.hpp"
#include "Utility.hpp"

namespace tako
{
	void Input::HandleEvent(Event& evt)
	{
		switch (evt.GetType()) {
			case tako::EventType::KeyPress:
			{
				tako::KeyPress& press = static_cast<tako::KeyPress &>(evt);
				if (press.key != Key::Unknown)
				{
					activeKeys[static_cast<size_t>(press.key)] = press.status;
				}
			} break;
			case tako::EventType::MouseMove:
			{
				tako::MouseMove& move = static_cast<tako::MouseMove&>(evt);
				m_mousePosition = move.position;
			} break;
			case tako::EventType::AxisUpdate:
			{
				tako::AxisUpdate& update = static_cast<tako::AxisUpdate&>(evt);
				m_axis[static_cast<size_t>(update.axis)] = update.value;
			} break;
		}
	}

	bool Input::GetKey(Key key)
	{
		return keys[static_cast<size_t>(key)] == KeyStatus::Down;
	}

	bool Input::GetKeyDown(Key key)
	{
		return keys[static_cast<size_t>(key)] == KeyStatus::Down && prevKeys[static_cast<size_t>(key)] == KeyStatus::Up;
	}

	bool Input::GetKeyUp(Key key)
	{
		return keys[static_cast<size_t>(key)] == KeyStatus::Up && prevKeys[static_cast<size_t>(key)] == KeyStatus::Down;
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

	void Input::Update()
	{
		prevKeys = keys;
		keys = activeKeys;
	}
}

