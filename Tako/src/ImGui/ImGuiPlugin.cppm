module;
#include "imgui.h"
export module Tako.ImGui;

import Tako.InputEvent;

namespace tako
{
	export class ImGuiInputHandler : public IInputEventHandler
	{
		bool HandleInputEvent(InputEvent& evt) override
		{
			auto& io = ImGui::GetIO();
			// Actual processing is done via imgui backends
			// Only determine if got captures
			switch (evt.GetType())
			{
				case InputEventType::KeyPress:
				case InputEventType::TextInputUpdate:
					return io.WantCaptureKeyboard;
				case InputEventType::MouseMove:
				case InputEventType::MouseButtonPress:
					return io.WantCaptureMouse;
				default: return false;
			}
		}
	};
}