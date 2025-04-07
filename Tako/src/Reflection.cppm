module;
#include <vector>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <format>
export module Tako.Reflection;

namespace tako::Reflection
{
	export enum class TypeKind
	{
		Primitive,
		Struct,
		Array
	};

	export struct TypeInformation
	{
		const char* name;
		size_t size;
		TypeKind kind;

		constexpr TypeInformation() {}

		template<typename T>
		constexpr bool IsType() const;
	};

	export struct PrimitiveInformation : public TypeInformation
	{
		constexpr PrimitiveInformation(const char* name, size_t size)
		{
			this->name = name;
			this->size = size;
			this->kind = TypeKind::Primitive;
		}
	};

	export template<typename T>
		const PrimitiveInformation* GetPrimitiveInformation();

	export struct StructInformation;

	namespace Detail
	{
		inline auto& GetTypeRegistry()
		{
			static std::unordered_map<std::string_view, const tako::Reflection::StructInformation*> registry;
			return registry;
		}
	}

	struct StructInformation : public TypeInformation
	{
		StructInformation(void (*init)(StructInformation*))
		{
			init(this);
			this->kind = TypeKind::Struct;
			Detail::GetTypeRegistry()[this->name] = this;
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

	export struct ArrayInformation : public TypeInformation
	{
		const TypeInformation* elementType;
		const void* (*GetData)(const void*);
		size_t(*GetSize)(const void*);
		void* (*Push)(void*);
	};

	export template<typename T>
		concept ReflectedStruct = requires(T t)
	{
		{ &T::Reflection } -> std::convertible_to<const StructInformation*>;
	};

	export template<typename T>
		concept ReflectedPrimitive = requires(T t)
	{
		{ GetPrimitiveInformation<T>() } -> std::same_as<const PrimitiveInformation*>;
	};

	template<typename T>
	concept NonVectorReflected = ReflectedStruct<T> || ReflectedPrimitive<T>;

	export template<typename T>
		concept ReflectedVector = requires(T t)
	{
		typename T::value_type;
		requires std::same_as<T, std::vector<typename T::value_type>>;
		requires NonVectorReflected<typename T::value_type>;
		//TODO: Also check for ReflectedVector<typename T::value_type>
	};

	export template<typename T>
		concept ReflectedType = NonVectorReflected<T> || ReflectedVector<T>;

	export struct Resolver
	{
		template<ReflectedType T>
		static constexpr auto Get()
		{
			if constexpr (ReflectedStruct<T>)
			{
				return &T::Reflection;
			}
			else if constexpr (ReflectedVector<T>)
			{
				static ArrayInformation info = []()
					{
						ArrayInformation info;
						auto elementType = Get<typename T::value_type>();
						static std::string typeName = std::format("vector<{}>", elementType->name);
						info.name = typeName.c_str();
						info.size = sizeof(T);
						info.kind = TypeKind::Array;
						info.elementType = elementType;
						info.GetData = [](const void* data) -> const void*
							{
								return reinterpret_cast<const T*>(data)->data();
							};
						info.GetSize = [](const void* data) -> size_t
							{
								return reinterpret_cast<const T*>(data)->size();
							};
						info.Push = [](void* data) -> void*
							{
								auto& vec = *reinterpret_cast<T*>(data);
								return &vec.emplace_back();
							};

						return info;
					}();
				return &info;
			}
			else if constexpr (ReflectedPrimitive<T>)
			{
				return GetPrimitiveInformation<T>();
			}
			else
			{
				// std::unreachable
			}
		}


		template<typename T>
		static constexpr const char* GetName()
		{
			return Get<T>()->name;
		}

		static const StructInformation* GetTypeByName(std::string_view name)
		{
			auto& registry = Detail::GetTypeRegistry();
			auto it = registry.find(name);
			return it != registry.end() ? it->second : nullptr;
		}
	};

	export template<typename T>
		constexpr bool TypeInformation::IsType() const
	{
		return this == Resolver::Get<T>();
	}

	export template<typename T>
	void InitDefault(void* data)
	{
		new (data) T();
	}

	export template<class T, typename F>
	struct MemberPtr
	{
		const char* name;
		F T::* memberPtr;
	};
}


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
DEFINE_REFLECTION_PRIMITIVE(unsigned char)
DEFINE_REFLECTION_PRIMITIVE(std::string) //TODO: handle properly
