module;
#include "Utility.hpp"
#include <RmlUi/Core/SystemInterface.h>
export module Tako.RmlUi.System;

namespace tako
{
export class RmlUiSystem : public Rml::SystemInterface
{
public:
	bool LogMessage(Rml::Log::Type type, const Rml::String& message) override
	{
		switch (type)
		{
			case Rml::Log::LT_INFO:
				LOG("[RmlUi] {}", message);
				break;
			case Rml::Log::LT_WARNING:
				LOG_WARN("[RmlUi] {}", message);
				break;
			case Rml::Log::LT_ERROR:
			case Rml::Log::LT_ASSERT:
				LOG_ERR("[RmlUi] {}", message);
				break;
			default:
				LOG("[RmlUi {}] {}", static_cast<int>(type), message);
				break;
		}

		return true;
	}
};

}
