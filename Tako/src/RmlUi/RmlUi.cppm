module;
#include "Utility.hpp"
#include "Reflection.hpp"
#include <RmlUi/Core.h>
#include <variant>
#include <mutex>
export module Tako.RmlUi;

import Tako.StringView;
import Tako.GraphicsContext;
import Tako.RmlUi.Renderer;
import Tako.RmlUi.System;
import Tako.Event;
import Tako.Window;
import Tako.NumberTypes;
import Tako.Resources;


namespace tako
{

Rml::Input::KeyIdentifier RmlConvertKey(Key key);
int RmlConvertMouseButton(MouseButton button);

export struct RmlDocument
{
	U64 value;
};

export class RmlUi : public IEventHandler
{
public:
	RmlUi()
	{}

	void Init(Window* window, GraphicsContext* graphicsContext)
	{
		m_window = window;
		m_graphicsContext = graphicsContext;
		m_renderer.Init(graphicsContext);

		Rml::SetRenderInterface(&m_renderer);
		Rml::SetSystemInterface(&m_system);

		Rml::Initialise();
		m_context = Rml::CreateContext("main", Rml::Vector2i(graphicsContext->GetWidth(), graphicsContext->GetHeight()));
		ASSERT(m_context);
	}

	void Update()
	{
		m_context->Update();
	}

	void Draw()
	{
		m_renderer.Begin();
		{
			m_context->SetDimensions({(int)m_graphicsContext->GetWidth(), (int)m_graphicsContext->GetHeight()});
			m_context->Render();
		}
		m_renderer.End();
	}

	void LoadFont(StringView filePath)
	{
		//TODO: Check if RmlUi could accept string_view/c_str
		Rml::String path(filePath);
		if (!Rml::LoadFontFace(path))
		{
			LOG_ERR("Error loading font {}", path);
		}
	}

	using ModelHandle = Rml::DataModelHandle;
	template<typename T>
	ModelHandle RegisterDataBinding(T& data)
	{
		LOG("{}", Reflection::Resolver::GetName<T>());
		auto constructor = m_context->CreateDataModel(Reflection::Resolver::GetName<T>());
		/*
		if (auto structHandle = constructor.template RegisterStruct<T>())
		{
			auto regMember = [&](auto& arg)
			{
				structHandle.RegisterMember(arg.name, arg.memberPtr);
			};
			std::apply([&](auto&... args) {((regMember(args)), ...);}, T::ReflectionMemberPtrs);
		}
		*/
		auto regMember = [&](auto& mem)
		{
			constructor.Bind(mem.name, &(data.*mem.memberPtr));
		};
		std::apply([&](auto&... args) {((regMember(args)), ...);}, T::ReflectionMemberPtrs);
		return constructor.GetModelHandle();
	}

	RmlDocument LoadDocument(StringView filePath)
	{
		Rml::String path(filePath);
		auto doc = m_context->LoadDocument(path);
		return std::bit_cast<RmlDocument>(doc);
	}

	void ReleaseDocument(RmlDocument document)
	{
		auto doc = std::bit_cast<Rml::ElementDocument*>(document);
		m_context->UnloadDocument(doc);
	}

	void ShowDocument(RmlDocument document)
	{
		auto doc = std::bit_cast<Rml::ElementDocument*>(document);
		doc->Show();
	}

	void RegisterLoaders(Resources* resources)
	{
		resources->RegisterLoader(this, &RmlUi::LoadDocument, &RmlUi::ReleaseDocument);
	}

	void HandleEvent(Event& evt) override
	{
		if (!m_context)
		{
			return;
		}

		switch (evt.GetType())
		{
			case EventType::KeyPress:
			{
				KeyPress& press = static_cast<KeyPress&>(evt);
				auto key = RmlConvertKey(press.key);
				int keyMod = 0;
				if (press.status == KeyStatus::Down)
				{
					m_context->ProcessKeyDown(key, keyMod);
				}
				else
				{
					m_context->ProcessKeyUp(key, keyMod);
				}
			} break;
			case EventType::TextInputUpdate:
			{
				TextInputUpdate& input = static_cast<TextInputUpdate&>(evt);
				std::visit(overloaded
				{
					[&](U32 character)
					{
						m_context->ProcessTextInput(character);
					},
					[&](const char* str)
					{
						m_context->ProcessTextInput(str);
					}
				}, input.input);

			} break;
			case EventType::MouseMove:
			{
				MouseMove& move = static_cast<MouseMove&>(evt);
				m_context->ProcessMouseMove(move.position.x, m_graphicsContext->GetHeight() - move.position.y, 0);
			} break;
			case EventType::MouseButtonPress:
			{
				MouseButtonPress& press = static_cast<MouseButtonPress&>(evt);
				int button = RmlConvertMouseButton(press.button);
				if (button < 0)
				{
					return;
				}
				int keyMod = 0;
				if (press.status == MouseButtonStatus::Down)
				{
					m_context->ProcessMouseButtonDown(button, keyMod);
				}
				else
				{
					m_context->ProcessMouseButtonUp(button, keyMod);
				}
			} break;
		}

	}

private:
	RmlUiRenderer m_renderer;
	RmlUiSystem m_system;
	Rml::Context* m_context = nullptr;
	Window* m_window = nullptr;
	GraphicsContext* m_graphicsContext = nullptr;
	std::mutex m_mutex;
};

Rml::Input::KeyIdentifier RmlConvertKey(Key key)
{
	switch (key)
	{
		case Key::A: return Rml::Input::KI_A;
		case Key::B: return Rml::Input::KI_B;
		case Key::C: return Rml::Input::KI_C;
		case Key::D: return Rml::Input::KI_D;
		case Key::E: return Rml::Input::KI_E;
		case Key::F: return Rml::Input::KI_F;
		case Key::G: return Rml::Input::KI_G;
		case Key::H: return Rml::Input::KI_H;
		case Key::I: return Rml::Input::KI_I;
		case Key::J: return Rml::Input::KI_J;
		case Key::K: return Rml::Input::KI_K;
		case Key::L: return Rml::Input::KI_L;
		case Key::M: return Rml::Input::KI_M;
		case Key::N: return Rml::Input::KI_N;
		case Key::O: return Rml::Input::KI_O;
		case Key::P: return Rml::Input::KI_P;
		case Key::Q: return Rml::Input::KI_Q;
		case Key::R: return Rml::Input::KI_R;
		case Key::S: return Rml::Input::KI_S;
		case Key::T: return Rml::Input::KI_T;
		case Key::U: return Rml::Input::KI_U;
		case Key::V: return Rml::Input::KI_V;
		case Key::W: return Rml::Input::KI_W;
		case Key::X: return Rml::Input::KI_X;
		case Key::Y: return Rml::Input::KI_Y;
		case Key::Z: return Rml::Input::KI_Z;
		case Key::Up: return Rml::Input::KI_UP;
		case Key::Down: return Rml::Input::KI_DOWN;
		case Key::Left: return Rml::Input::KI_LEFT;
		case Key::Right: return Rml::Input::KI_RIGHT;
		case Key::Space: return Rml::Input::KI_SPACE;
		case Key::Enter: return Rml::Input::KI_RETURN;
		case Key::Backspace: return Rml::Input::KI_BACK;
	}

	return Rml::Input::KI_UNKNOWN;
}

int RmlConvertMouseButton(MouseButton button)
{
	switch (button)
	{
		case MouseButton::Left: return 0;
		case MouseButton::Right: return 1;
		case MouseButton::Middle: return 2;
	}

	return -1;
}

}
