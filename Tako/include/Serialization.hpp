#pragma once
#include "Utility.hpp"
#include "FileSystem.hpp"
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

	static void TestYaml()
	{
		auto path = FileSystem::GetExecutablePath() + "/testComp.yaml";
		auto comp = Deserialize<TestComponent>(FileSystem::ReadText(path.c_str()).c_str());
		LOG("{} {} {} {} {} {} {}", comp.index, comp.type, comp.flying, comp.trample, comp.haste, comp.pos.x, comp.pos.y);
		comp.trample = !comp.trample;
		comp.index *= 2;
		auto node = Serialize(comp);
		LOG("{}", node);
	}
}
