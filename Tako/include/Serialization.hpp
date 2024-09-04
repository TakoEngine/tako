#pragma once
#include "Utility.hpp"
#include "Reflection.hpp"
#include <type_traits>

namespace tako::Serialization
{
	struct SubComp
	{
		int x;
		int y;
	private:
		REFLECT()
	};

	struct TestComponent
	{
		int index;
		int type;
		bool flying;
		bool trample;
		bool haste;
		SubComp pos;
	private:
		REFLECT()
	};

	void Decode(const char* text, void* data, const Reflection::StructInformation* info);

	template<typename T, std::enable_if_t<Reflection::Resolver::IsReflected<T>::value, bool> = true>
	T Deserialize(const char* text)
	{
		T t = {};
		Decode(text, &t, Reflection::Resolver::Get<T>());
		return t;
	}

	std::string Encode(const void* data, const Reflection::StructInformation* info);

	template<typename T, std::enable_if_t<Reflection::Resolver::IsReflected<T>::value, bool> = true>
	std::string Serialize(const T& t)
	{
		return Encode(&t, Reflection::Resolver::Get<T>());
	}
}
