module;
#include "GraphicsAPI.hpp"
#include "WindowHandle.hpp"
#include <functional>
#include <memory>
export module Tako.Window;

export import Tako.Event;
export import Tako.InputEvent;
import Tako.Math;

namespace tako
{
	export class Window
	{
	public:
		Window(GraphicsAPI api);
		~Window();
		void Poll();
		bool ShouldExit();
		int GetWidth();
		int GetHeight();
		Point GetFramebufferSize();
		WindowHandle GetHandle() const;

		void SetEventCallback(const std::function<void(Event&)>& callback);
		void SetInputCallback(const std::function<bool(InputEvent&)>& callback);

		enum class FullScreenMode
		{
			Windowed,
			FullScreen
		};
		void SetFullScreenMode(FullScreenMode mode);
		FullScreenMode GetFullScreenMode();

	private:
		class WindowImpl;
		std::unique_ptr<WindowImpl> m_impl;
	};
}
