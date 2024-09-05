module;
#include "Reflection.hpp"
#include "Utility.hpp"
#include <yaml-cpp/yaml.h>
#include <type_traits>
export module Tako.Serialization;

import Tako.NumberTypes;

namespace tako::Serialization
{
	export void Decode(const char* text, void* data, const Reflection::StructInformation* info);

	export template<typename T, std::enable_if_t<Reflection::Resolver::IsReflected<T>::value, bool> = true>
	T Deserialize(const char* text)
	{
		T t = {};
		Decode(text, &t, Reflection::Resolver::Get<T>());
		return t;
	}

	export std::string Encode(const void* data, const Reflection::StructInformation* info);

	export template<typename T, std::enable_if_t<Reflection::Resolver::IsReflected<T>::value, bool> = true>
	std::string Serialize(const T& t)
	{
		return Encode(&t, Reflection::Resolver::Get<T>());
	}

	void EmitPrimitive(const void* data, const Reflection::StructInformation::Field* info, ::YAML::Emitter& out)
	{
		out << ::YAML::Value;
		if (Reflection::GetPrimitiveInformation<int>() == info->type)
		{
			out << *reinterpret_cast<const int*>(reinterpret_cast<const U8*>(data) + info->offset);
		}
		else if (Reflection::GetPrimitiveInformation<bool>() == info->type)
		{
			out << *reinterpret_cast<const bool*>(reinterpret_cast<const U8*>(data) + info->offset);
		}
	}

	void EmitMap(const void* data, const Reflection::StructInformation* info, ::YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		for (auto& descriptor : info->fields)
		{
			out << ::YAML::Key << descriptor.name;
			if (auto strInfo = dynamic_cast<const Reflection::StructInformation*>(descriptor.type))
			{
				EmitMap(reinterpret_cast<const U8*>(data) + descriptor.offset, strInfo, out);
			}
			else
			{
				EmitPrimitive(data, &descriptor, out);
			}

		}
		out << YAML::EndMap;
	}

	std::string Encode(const void* data, const Reflection::StructInformation* info)
	{
		::YAML::Emitter out;
		EmitMap(data, info, out);
		return { out.c_str() };
	}

	void AssignPrimitive(void* data, const Reflection::StructInformation::Field* info, const ::YAML::Node& node)
	{
		if (Reflection::GetPrimitiveInformation<int>() == info->type)
		{
			*reinterpret_cast<int*>(reinterpret_cast<U8*>(data) + info->offset) = node[info->name].as<int>();
		}
		else if (Reflection::GetPrimitiveInformation<bool>() == info->type)
		{
			*reinterpret_cast<bool*>(reinterpret_cast<U8*>(data) + info->offset) = node[info->name].as<bool>();
		}
	}

	void AssignMap(void* data, const Reflection::StructInformation* info, const ::YAML::Node& node)
	{
		for (auto& descriptor : info->fields)
		{
			if (auto strInfo = dynamic_cast<const Reflection::StructInformation*>(descriptor.type))
			{
				auto strData = reinterpret_cast<U8*>(data) + descriptor.offset;
				strInfo->constr(strData);
				AssignMap(strData, strInfo, node[descriptor.name]);
			}
			else
			{
				AssignPrimitive(data, &descriptor, node);
			}
		}
	}

	void Decode(const char* text, void* data, const Reflection::StructInformation* info)
	{
		auto node = YAML::Load(text);
		AssignMap(data, info, node);
	}
}
