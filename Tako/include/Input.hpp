#pragma once
#include "Event.hpp"
#include "Math.hpp"
#include <array>

namespace tako
{
	class Input : public IEventHandler
	{
	public:
		virtual void HandleEvent(Event& evt) override;
		void Update();
		bool GetKey(Key key);
		bool GetKeyDown(Key key);
		bool GetKeyUp(Key key);
		Vector2 GetMousePosition();
	private:
		Vector2 m_mousePosition;
		std::array<KeyStatus, static_cast<size_t>(Key::Unknown)> activeKeys = {KeyStatus::Up};
		std::array<KeyStatus, static_cast<size_t>(Key::Unknown)> keys = {KeyStatus::Up};
		std::array<KeyStatus, static_cast<size_t>(Key::Unknown)> prevKeys = {KeyStatus::Up};
	};
}
