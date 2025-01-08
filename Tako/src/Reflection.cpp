#include "Reflection.hpp"
#include <string>

#define DEFINE_REFLECTION_PRIMITIVE(type) \
	template<> \
	const ::tako::Reflection::PrimitiveInformation* ::tako::Reflection::GetPrimitiveInformation<type>() \
	{ \
		static ::tako::Reflection::PrimitiveInformation info(#type, sizeof(type)); \
		return &info; \
	}

DEFINE_REFLECTION_PRIMITIVE(int)
DEFINE_REFLECTION_PRIMITIVE(float)
DEFINE_REFLECTION_PRIMITIVE(bool)
DEFINE_REFLECTION_PRIMITIVE(std::string) //TODO: handle properly
