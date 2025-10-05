module;
#include "Utility.hpp"
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
import Tako.HandleVec;
import Tako.Reflection;


namespace tako
{

Rml::Input::KeyIdentifier RmlConvertKey(Key key);
int RmlConvertMouseButton(MouseButton button);

export struct RmlDocument
{
	U64 value;
};

export struct RmlDataModel
{
	U64 value;

	void DirtyVariable(StringView name)
	{
		handle.DirtyVariable(Rml::String(name));
	}
private:
	friend class RmlUi;
	Rml::DataModelHandle handle;
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

	template<typename T>
	RmlDataModel RegisterDataModel(T& data, StringView modelName)
	{
		ModelEntry entry;
		entry.name = modelName;
		auto constructor = m_context->CreateDataModel(entry.name);
		auto typeRegister = constructor.GetDataTypeRegister();

		auto regMember = [&](auto& mem)
		{
			using FieldType = typename std::decay_t<decltype(mem)>::FieldType;
			EnsureTypeIsRegistered<FieldType>(constructor, typeRegister);
			constructor.Bind(mem.name, &(data.*mem.memberPtr));
		};
		std::apply([&](auto&... args) {((regMember(args)), ...);}, T::ReflectionMemberPtrs);
		auto modelHandle = entry.handle = constructor.GetModelHandle();
		auto handle = m_dataModels.Insert(std::move(entry));
		handle.handle = modelHandle;
		return handle;
	}

	template<typename T>
	RmlDataModel RegisterDataModel(T& data)
	{
		return RegisterDataModel(data, Reflection::Resolver::GetName<T>());
	}

	template<typename... Args, std::invocable<Args...> Callback>
	void RegisterCallback(RmlDataModel dataModel, const std::string& callbackName, Callback callback)
	{
		auto& model = m_dataModels[dataModel];
		auto constructor = m_context->GetDataModel(model.name);
		auto rmlCallback = [callback, callbackName](Rml::DataModelHandle handle, Rml::Event& event, const Rml::VariantList& variantList)
		{
			constexpr size_t argCount = sizeof...(Args);
			auto variantCount = variantList.size();
			if (variantCount < argCount)
			{
				LOG_ERR("RmlCallback ({}) called with less arguments ({}) than required {}", callbackName, variantCount, argCount);
				return;
			}

			auto convertArgs = [&]<std::size_t... I>(std::index_sequence<I...>)
			{
				return std::tuple{ variantList[I].Get<Args>()... };
			};

			std::apply(callback, convertArgs(std::make_index_sequence<argCount>{}));
		};
		constructor.BindEventCallback(callbackName, rmlCallback);
	}

	void ReleaseDataModel(RmlDataModel dataModel)
	{
		auto& model = m_dataModels[dataModel];
		m_context->RemoveDataModel(model.name);
	}

	RmlDocument LoadDocument(StringView filePath)
	{
		Rml::String path(filePath);
		DocumentEntry entry;
		entry.doc = m_context->LoadDocument(path);
		return m_documents.Insert(std::move(entry));
	}

	void ReleaseDocument(RmlDocument document)
	{
		auto& entry = m_documents[document];
		m_context->UnloadDocument(entry.doc);
		m_documents.Remove(document);
	}

	void ReloadDocument(RmlDocument old, const StringView filePath)
	{
		auto& entry = m_documents[old];
		m_context->UnloadDocument(entry.doc);
		Rml::String path(filePath);
		entry.doc = m_context->LoadDocument(path);
		if (entry.shown)
		{
			entry.doc->Show();
		}
	}

	void ShowDocument(RmlDocument document)
	{
		auto& entry = m_documents[document];
		entry.shown = true;
		entry.doc->Show();
	}

	void HideDocument(RmlDocument document)
	{
		auto& entry = m_documents[document];
		entry.shown = false;
		entry.doc->Hide();
	}

	void RegisterLoaders(Resources* resources)
	{
		resources->RegisterLoader(this, &RmlUi::LoadDocument, &RmlUi::ReleaseDocument, &RmlUi::ReloadDocument);
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
	struct DocumentEntry
	{
		Rml::ElementDocument* doc = nullptr;
		bool shown = false;
	};

	struct ModelEntry
	{
		Rml::DataModelHandle handle;
		std::string name;
	};

	RmlUiRenderer m_renderer;
	RmlUiSystem m_system;
	Rml::Context* m_context = nullptr;
	Window* m_window = nullptr;
	HandleVec<RmlDocument, DocumentEntry> m_documents;
	HandleVec<RmlDataModel, ModelEntry> m_dataModels;
	GraphicsContext* m_graphicsContext = nullptr;
	std::mutex m_mutex;

	template<typename T>
	void EnsureTypeIsRegistered(Rml::DataModelConstructor& constructor, Rml::DataTypeRegister* typeRegister)
	{
		//TODO: Figure out if RmlUI has a way to check for unregistered types without triggering an error log
		if (!typeRegister->GetDefinition<T>())
		{
			RegisterType<T>(constructor, typeRegister);
		}
	}

	template<typename T>
	void RegisterType(Rml::DataModelConstructor& constructor, Rml::DataTypeRegister* typeRegister)
	{
		if constexpr (Reflection::ReflectedStruct<T>)
		{
			auto handle = constructor.RegisterStruct<T>();
			ASSERT(handle);
			auto regMember = [&](auto& mem)
			{
				using FieldType = typename std::decay_t<decltype(mem)>::FieldType;
				EnsureTypeIsRegistered<FieldType>(constructor, typeRegister);
				handle.RegisterMember(mem.name, mem.memberPtr);
			};
			std::apply([&](auto&... args) {((regMember(args)), ...); }, T::ReflectionMemberPtrs);
			return;
		}
		else if constexpr (Reflection::ReflectedVector<T>)
		{
			EnsureTypeIsRegistered<typename T::value_type>(constructor, typeRegister);
			constructor.RegisterArray<T>();
		}
	}
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
