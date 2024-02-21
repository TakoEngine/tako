#pragma once
#include "GraphicsAPI.hpp"
#include "WindowHandle.hpp"
//#include <memory>
#include "Event.hpp"
#include <functional>

namespace tako
{
	enum class GraphicsAPI;

	class Window
	{
	public:
		Window(GraphicsAPI api);
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
