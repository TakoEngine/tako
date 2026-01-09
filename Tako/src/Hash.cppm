module;
#pragma push_macro("constexpr")
#ifndef constexpr
	#define constexpr 
#endif
#include <rapidhash.h>
#pragma pop_macro("constexpr")
#include <string>
#include <string_view>
export module Tako.Hash;

import Tako.NumberTypes;
import Tako.Reflection;
import Tako.Serialization;

namespace tako::Hash
{
	export U64 HashStr(std::string_view str)
	{
		return rapidhash(str.data(), str.size());
	}

	export template<Reflection::ReflectedType T>
	U64 HashReflected(const T& t)
	{
		//TODO: do proper hashing instead of doing yaml
		auto str = tako::Serialization::YAML::Serialize(t);
		return HashStr(str);
	}
}
