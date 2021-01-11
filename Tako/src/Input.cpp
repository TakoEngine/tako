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

	void Input::Update()
	{
		prevKeys = keys;
		keys = activeKeys;
	}
}

