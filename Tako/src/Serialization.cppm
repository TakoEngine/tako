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

	void Emit(const void* data, const Reflection::TypeInformation* info, ::YAML::Emitter& out);

	void EmitPrimitive(const void* data, const Reflection::PrimitiveInformation* info, ::YAML::Emitter& out)
	{
		out << ::YAML::Value;
		if (Reflection::GetPrimitiveInformation<int>() == info)
		{
			out << *reinterpret_cast<const int*>(data);
		}
		else if (Reflection::GetPrimitiveInformation<float>() == info)
		{
			out << *reinterpret_cast<const float*>(data);
		}
		else if (Reflection::GetPrimitiveInformation<bool>() == info)
		{
			out << *reinterpret_cast<const bool*>(data);
		}
		else if (Reflection::GetPrimitiveInformation<std::string>() == info)
		{
			out << *reinterpret_cast<const std::string*>(data);
		}
	}

	void EmitMap(const void* data, const Reflection::StructInformation* info, ::YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		for (auto& descriptor : info->fields)
		{
			out << ::YAML::Key << descriptor.name;
			auto fieldData = reinterpret_cast<const void*>(reinterpret_cast<const U8*>(data) + descriptor.offset);
			Emit(fieldData, descriptor.type, out);
		}
		out << YAML::EndMap;
	}

	void EmitArray(const void* data, const Reflection::ArrayInformation* info, ::YAML::Emitter& out)
	{
		auto size = info->GetSize(data);
		auto arrData = info->GetData(data);
		size_t elementSize = info->elementType->size;
		out << YAML::BeginSeq;
		for (size_t i = 0; i < size; ++i)
		{
			auto elementPtr = reinterpret_cast<const U8*>(arrData) + (i * elementSize);
			Emit(elementPtr, info->elementType, out);
		}
		out << YAML::EndSeq;
	}

	void Emit(const void* data, const Reflection::TypeInformation* info, ::YAML::Emitter& out)
	{
		switch (info->kind)
		{
			case Reflection::TypeKind::Struct:
			{
				auto strInfo = reinterpret_cast<const Reflection::StructInformation*>(info);
				EmitMap(data, strInfo, out);
			} break;
			case Reflection::TypeKind::Array:
			{
				auto arrInfo = reinterpret_cast<const Reflection::ArrayInformation*>(info);
				EmitArray(data, arrInfo, out);
			} break;
			case Reflection::TypeKind::Primitive:
			{
				auto primInfo = reinterpret_cast<const Reflection::PrimitiveInformation*>(info);
				EmitPrimitive(data, primInfo, out);
			} break;
		}
	}

	std::string Encode(const void* data, const Reflection::StructInformation* info)
	{
		::YAML::Emitter out;
		EmitMap(data, info, out);
		return { out.c_str() };
	}

	void Assign(void* data, const Reflection::TypeInformation* info, const ::YAML::Node& node);

	void AssignPrimitive(void* data, const Reflection::PrimitiveInformation* info, const ::YAML::Node& node)
	{
		if (info->IsType<int>())
		{
			*reinterpret_cast<int*>(data) = node.as<int>();
		}
		else if (info->IsType<float>())
		{
			*reinterpret_cast<float*>(data) = node.as<float>();
		}
		else if (info->IsType<bool>())
		{
			*reinterpret_cast<bool*>(data) = node.as<bool>();
		}
		else if (info->IsType<std::string>())
		{
			*reinterpret_cast<std::string*>(data) = node.as<std::string>();
		}
	}

	void AssignMap(void* data, const Reflection::StructInformation* info, const ::YAML::Node& node)
	{
		for (auto& descriptor : info->fields)
		{
			auto fieldData = reinterpret_cast<U8*>(data) + descriptor.offset;
			Assign(fieldData, descriptor.type, node[descriptor.name]);
		}
	}

	void AssignArray(void* data, const Reflection::ArrayInformation* info, const ::YAML::Node& node)
	{
		auto size = node.size();
		for (size_t i = 0; i < size; ++i)
		{
			auto newElement = info->Push(data);
			Assign(newElement, info->elementType, node[i]);
		}
	}

	void Assign(void* data, const Reflection::TypeInformation* info, const ::YAML::Node& node)
	{
		switch (info->kind)
		{
		case Reflection::TypeKind::Struct:
		{
			auto strInfo = reinterpret_cast<const Reflection::StructInformation*>(info);
			AssignMap(data, strInfo, node);
		} break;
		case Reflection::TypeKind::Array:
		{
			auto arrInfo = reinterpret_cast<const Reflection::ArrayInformation*>(info);
			AssignArray(data, arrInfo, node);
		} break;
		case Reflection::TypeKind::Primitive:
		{
			auto primInfo = reinterpret_cast<const Reflection::PrimitiveInformation*>(info);
			AssignPrimitive(data, primInfo, node);
		} break;
		}
	}

	void Decode(const char* text, void* data, const Reflection::StructInformation* info)
	{
		auto node = YAML::Load(text);
		AssignMap(data, info, node);
	}
}
