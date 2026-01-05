module;
#include "Utility.hpp"
#include <yaml-cpp/yaml.h>
#include <type_traits>
export module Tako.Serialization;

import Tako.NumberTypes;
import Tako.Reflection;
import Tako.StringView;

namespace tako::Serialization::YAML
{
	export template<typename T>
		concept CustomYAMLSerializer = requires (T t)
	{
		{ t.Serialize(std::declval<::YAML::Emitter&>()) };
		{ t.Deserialize(std::declval<const ::YAML::Node&>()) };
	};

	export template<typename T>
		concept YAMLSerializable = CustomYAMLSerializer<T> || Reflection::ReflectedType<T>;

	export void Decode(void* target, const Reflection::TypeInformation* info, const ::YAML::Node& node);

	export template<Reflection::ReflectedType T>
		requires (!CustomYAMLSerializer<T>)
	void Decode(T& target, const ::YAML::Node& node)
	{
		Decode(&target, Reflection::Resolver::Get<T>(), node);
	}

	export template<CustomYAMLSerializer T>
	void Decode(T& target, const ::YAML::Node& node)
	{
		target.Deserialize(node);
	}

	export template<YAMLSerializable T>
	T Deserialize(StringView text)
	{
		auto textBuffer = tako::CStringBuffer(text);
		auto node = ::YAML::Load(textBuffer.c_str());
		T t = {};
		Decode(t, node);
		return t;
	}

	export void Encode(const void* data, const Reflection::TypeInformation* info, ::YAML::Emitter& out);

	export template<Reflection::ReflectedType T>
		requires (!CustomYAMLSerializer<T>)
	void Encode(const T& source, ::YAML::Emitter& out)
	{
		Encode(&source, Reflection::Resolver::Get<T>(), out);
	}

	export template<CustomYAMLSerializer T>
	void Encode(const T& source, ::YAML::Emitter& out)
	{
		source.Serialize(out);
	}

	export template<YAMLSerializable T>
	std::string Serialize(const T& t)
	{
		::YAML::Emitter out;
		Encode(t, out);
		return { out.c_str() };;
	}

	export std::string Serialize(const void* data, const Reflection::TypeInformation* info)
	{
		::YAML::Emitter out;
		Encode(data, info, out);
		return { out.c_str() };;
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
		else if (info->IsType<unsigned char>())
		{
			out << *reinterpret_cast<const unsigned char*>(data);
		}
		else if (info->IsType<tako::U16>())
		{
			out << *reinterpret_cast<const tako::U16*>(data);
		}
		else if (info->IsType<tako::U64>())
		{
			out << *reinterpret_cast<const tako::U64*>(data);
		}
		else if (Reflection::GetPrimitiveInformation<std::string>() == info)
		{
			out << *reinterpret_cast<const std::string*>(data);
		}
	}

	void EmitEnum(const void* data, const Reflection::EnumInformation* info, ::YAML::Emitter& out)
	{
		out << ::YAML::Value;
		size_t enumValue = info->convertUnderlying(data);
		for (auto& caseInfo : info->cases)
		{
			if (caseInfo.value == enumValue)
			{
				out << caseInfo.name;
				return;
			}
		}

		out << enumValue;
	}

	void EmitMap(const void* data, const Reflection::StructInformation* info, ::YAML::Emitter& out)
	{
		out << ::YAML::BeginMap;
		for (auto& descriptor : info->fields)
		{
			out << ::YAML::Key << descriptor.name;
			auto fieldData = reinterpret_cast<const void*>(reinterpret_cast<const U8*>(data) + descriptor.offset);
			Emit(fieldData, descriptor.type, out);
		}
		out << ::YAML::EndMap;
	}

	void EmitArray(const void* data, const Reflection::ArrayInformation* info, ::YAML::Emitter& out)
	{
		auto size = info->GetSize(data);
		auto arrData = info->GetData(data);
		size_t elementSize = info->elementType->size;
		out << ::YAML::BeginSeq;
		for (size_t i = 0; i < size; ++i)
		{
			auto elementPtr = reinterpret_cast<const U8*>(arrData) + (i * elementSize);
			Emit(elementPtr, info->elementType, out);
		}
		out << ::YAML::EndSeq;
	}

	void EmitPoly(const void* data, const Reflection::PolymorphicInformation* info, ::YAML::Emitter& out)
	{
		auto elementInfo = info->GetDerivedInfo(data); //TODO: How to get from polymorphic subclass?
		out << ::YAML::BeginMap;
		out << ::YAML::Key << "type";
		out << ::YAML::Value << elementInfo->name;
		out << ::YAML::Key << "data";
		Emit(static_cast<const std::unique_ptr<void>*>(data)->get(), elementInfo, out);
		out << ::YAML::EndMap;
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
		case Reflection::TypeKind::Enum:
		{
			auto enumInfo = reinterpret_cast<const Reflection::EnumInformation*>(info);
			EmitEnum(data, enumInfo, out);
		} break;
		case Reflection::TypeKind::Polymorphic:
		{
			auto polyInfo = reinterpret_cast<const Reflection::PolymorphicInformation*>(info);
			EmitPoly(data, polyInfo, out);
		} break;
		}
	}

	void Encode(const void* data, const Reflection::TypeInformation* info, ::YAML::Emitter& out)
	{
		Emit(data, info, out);
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
		else if (info->IsType<unsigned char>())
		{
			*reinterpret_cast<unsigned char*>(data) = node.as<unsigned char>();
		}
		else if (info->IsType<tako::U16>())
		{
			*reinterpret_cast<tako::U16*>(data) = node.as<tako::U16>();
		}
		else if (info->IsType<tako::U64>())
		{
			*reinterpret_cast<tako::U64*>(data) = node.as<tako::U64>();
		}
		else if (info->IsType<std::string>())
		{
			*reinterpret_cast<std::string*>(data) = node.as<std::string>();
		}
	}

	void AssignEnum(void* data, const Reflection::EnumInformation* info, const ::YAML::Node& node)
	{
		//TODO: Handle number nodes
		std::string caseName = node.as<std::string>();
		for (auto& caseInfo : info->cases)
		{
			if (caseName == caseInfo.name)
			{
				info->assignUnderlying(caseInfo.value, data);
			}
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

	void AssignPoly(void* data, const Reflection::PolymorphicInformation* info, const ::YAML::Node& node)
	{
		auto elementInfo = tako::Reflection::Resolver::GetTypeByName(node["type"].as<std::string>());
		auto elementData = elementInfo->ConstructNew();
		AssignMap(elementData, elementInfo, node["data"]);
		info->Reset(data, elementData);
	}

	void Assign(void* data, const Reflection::TypeInformation* info, const ::YAML::Node& node)
	{
		if (!node.IsDefined())
		{
			return;
		}
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
		case Reflection::TypeKind::Enum:
		{
			auto enumInfo = reinterpret_cast<const Reflection::EnumInformation*>(info);
			AssignEnum(data, enumInfo, node);
		} break;
		case Reflection::TypeKind::Polymorphic:
		{
			auto polyInfo = reinterpret_cast<const Reflection::PolymorphicInformation*>(info);
			AssignPoly(data, polyInfo, node);
		} break;
		}
	}

	void Decode(void* target, const Reflection::TypeInformation* info, const ::YAML::Node& node)
	{
		Assign(target, info, node);
	}
}
