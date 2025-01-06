module;
#include "Utility.hpp"
//#include "RmlUi_Platform_GLFW.h"
#include <RmlUi/Core.h>
export module Tako.RmlUi;

import Tako.StringView;
import Tako.GraphicsContext;
import Tako.RmlUi.Renderer;
import Tako.Window;

namespace tako
{

export class RmlUi
{
public:
	RmlUi()
	{}

	void Init(Window* window, GraphicsContext* graphicsContext)
	{
		m_graphicsContext = graphicsContext;
		m_renderer.Init(graphicsContext);
		//m_system.SetWindow(window->GetHandle());

		Rml::SetRenderInterface(&m_renderer);
		//Rml::SetSystemInterface(&m_system);

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
		ASSERT(Rml::LoadFontFace(path));
	}

	void LoadDocument(StringView filePath)
	{
		Rml::String path(filePath);
		m_doc = m_context->LoadDocument(path);
		if (m_doc)
		{
			LOG("Show doc");
			m_doc->Show();
		}
	}

private:
	RmlUiRenderer m_renderer;
	//SystemInterface_GLFW m_system;
	Rml::Context* m_context;
	GraphicsContext* m_graphicsContext;
	Rml::ElementDocument* m_doc;
	std::mutex m_mutex;
};

}
