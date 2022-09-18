#include "Serialization.hpp"
#include <yaml-cpp/yaml.h>

namespace tako::Serialization
{
	REFLECT_START(TestComponent)
		REFLECT_FIELD(index)
		REFLECT_FIELD(type)
		REFLECT_FIELD(flying)
		REFLECT_FIELD(trample)
		REFLECT_FIELD(haste)
	REFLECT_END()

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
			EmitPrimitive(data, &descriptor, out);
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

	void Decode(const char* text, void* data, const Reflection::StructInformation* info)
	{
		auto node = YAML::Load(text);
		for (auto& descriptor : info->fields)
		{
			AssignPrimitive(data, &descriptor, node);
		}
	}
	/*
	static T Decode(const ::YAML::Node& node)
	{
		T rhs;
		for (auto descriptor : Reflection::Resolver::Get<T>()->fields)
		{
			LOG("{}", descriptor.name);
			AssignPrimitive(rhs, &descriptor, node);
		}
		return rhs;
	}

	static void AssignPrimitive(T& rhs, const Reflection::StructInformation::Field* info, const ::YAML::Node& node)
	{
		if (Reflection::GetPrimitiveInformation<int>() == info->type)
		{
			*reinterpret_cast<int*>(reinterpret_cast<U8*>(&rhs) + info->offset) = node[info->name].as<int>();
		}
		else if (Reflection::GetPrimitiveInformation<bool>() == info->type)
		{
			*reinterpret_cast<bool*>(reinterpret_cast<U8*>(&rhs) + info->offset) = node[info->name].as<bool>();
		}
	}
	*/
}
