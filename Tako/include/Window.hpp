#pragma once
#include "WindowHandle.hpp"
#include <memory>
#include "Event.hpp"
#include <functional>

namespace tako
{
	class Window
	{
	public:
		Window();
		~Window();
		void Poll();
		bool ShouldExit();
		int GetWidth();
		int GetHeight();
		WindowHandle GetHandle() const;
		void SetEventCallback(const std::function<void(Event&)>& callback);
	private:
		class WindowImpl;
		std::unique_ptr<WindowImpl> m_impl;
	};
}
