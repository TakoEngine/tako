# pragma once
#include <vector>
#include <cstddef>

namespace tako::Reflection
{
	struct TypeInformation
	{
		const char* name;
		size_t size;

		virtual ~TypeInformation() {}
	};

	struct PrimitiveInformation : public TypeInformation
	{
		PrimitiveInformation(const char* name, size_t size)
		{
			this->name = name;
			this->size = size;
		}
	};

	template<typename T>
	const PrimitiveInformation* GetPrimitiveInformation();

	struct StructInformation : public TypeInformation
	{
		StructInformation(void (*init)(StructInformation*))
		{
			init(this);
		}

		struct Field
		{
			const char* name;
			size_t offset;
			const TypeInformation* type;
		};

		std::vector<Field> fields;
		void (*constr)(void*);
	};

	struct Resolver
	{
		template<typename T>
		static char func(decltype(&T::Reflection));
		template<typename T>
		static int func(...);

		template<typename T>
		struct IsReflected
		{
			enum { value = (sizeof(func<T>(nullptr)) == sizeof(char)) };
		};

		template<typename T>
		static const auto Get()
		{
			if constexpr (IsReflected<T>::value)
			{
				return &T::Reflection;
			}
			else
			{
				return GetPrimitiveInformation<T>();
			}
		}
	};

	template<typename T>
	static void InitDefault(void* data)
	{
		new (data) T();
	}
}


#define REFLECT() \
	friend struct ::tako::Reflection::Resolver; \
	static const ::tako::Reflection::StructInformation Reflection; \
	static void InitReflection(::tako::Reflection::StructInformation* info);

#define REFLECT_START(type) \
	const ::tako::Reflection::StructInformation type::Reflection{type::InitReflection}; \
	void type::InitReflection(::tako::Reflection::StructInformation* info) \
	{ \
		using T = type; \
		info->name = #type; \
		info->size = sizeof(T); \
		info->constr = &::tako::Reflection::InitDefault<T>; \
		info->fields = \
		{

#define REFLECT_FIELD(name) \
			{#name, offsetof(T, name), ::tako::Reflection::Resolver::Get<decltype(T::name)>()},

#define REFLECT_END() \
		}; \
	}
