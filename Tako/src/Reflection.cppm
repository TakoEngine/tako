module;
#include <vector>
#include <memory>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <span>
#include "fmt/format.h"
export module Tako.Reflection;

import Tako.StringView;
import Tako.NumberTypes;

namespace tako::Reflection
{
	export enum class TypeKind
	{
		Primitive,
		Struct,
		Enum,
		Array,
		Polymorphic
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
		void* (*ConstructNew)();
	};

	export struct EnumInformation : public TypeInformation
	{
		struct EnumCase
		{
			const char* name;
			std::size_t value;
		};

		EnumInformation(void (*init)(EnumInformation*))
		{
			init(this);
			this->kind = TypeKind::Enum;
		}

		std::span<EnumCase> cases;
		size_t (*convertUnderlying)(const void*);
		void (*assignUnderlying)(size_t, void*);
		//const TypeInformation* underlying;
	};

	export template<typename T>
	const EnumInformation* GetEnumInformation();

	export struct ArrayInformation : public TypeInformation
	{
		const TypeInformation* elementType;
		const void* (*GetData)(const void*);
		size_t(*GetSize)(const void*);
		void* (*Push)(void*);
	};

    export struct PolymorphicInformation : public TypeInformation
	{
		PolymorphicInformation(void (*init)(PolymorphicInformation*))
		{
			init(this);
			this->kind = TypeKind::Polymorphic;
			//TODO: Register TypeRegistry?
		}

		const TypeInformation* (*GetDerivedInfo)(const void*);
		void (*Reset)(void*, void*);
	};

	export template<typename T>
	concept ReflectedStruct = requires
	{
		{ &T::Reflection } -> std::convertible_to<const StructInformation*>;
	};

	export template<typename T>
	concept ReflectedPolymorphic = requires
	{
		typename T::element_type;
		requires std::same_as<T, std::unique_ptr<typename T::element_type>>;
		{ &T::element_type::Reflection } -> std::convertible_to<const PolymorphicInformation*>;
	};

	export template<typename T>
	concept ReflectedEnum = requires
	{
		{ GetEnumInformation<T>() } -> std::same_as<const EnumInformation*>;
	} && std::is_enum_v<T>;

	export template<typename T>
	concept ReflectedPrimitive = requires
	{
		{ GetPrimitiveInformation<T>() } -> std::same_as<const PrimitiveInformation*>;
	} && !std::is_enum_v<T>;

	template<typename T>
	concept NonVectorReflected = ReflectedStruct<T> || ReflectedPolymorphic<T> || ReflectedEnum<T> || ReflectedPrimitive<T>;

	export template<typename T>
		concept ReflectedVector = requires
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
			else if constexpr (ReflectedPolymorphic<T>)
			{
				return &T::element_type::Reflection;
			}
			else if constexpr (ReflectedVector<T>)
			{
				static ArrayInformation info = []()
					{
						ArrayInformation info;
						auto elementType = Get<typename T::value_type>();
						static std::string typeName = fmt::format("vector<{}>", elementType->name ? elementType->name : "");
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
			else if constexpr (ReflectedEnum<T>)
			{
				return GetEnumInformation<T>();
			}
			else if constexpr (ReflectedPrimitive<T>)
			{
				return GetPrimitiveInformation<T>();
			}
			else
			{
				// std::unreachable
				static_assert(false, "Tried to reflect unsupported Type");
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

		template<ReflectedEnum E>
		static constexpr const char* EnumName(E enm)
		{
			auto info = GetEnumInformation<E>();
			size_t enumValue = info->convertUnderlying(&enm);

			for (auto& caseInfo : info->cases)
			{
				if (caseInfo.value == enumValue)
				{ 
					return caseInfo.name;
				}
			}

			return nullptr;
		}
	};

	template<typename T>
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

		using ClassType = T;
		using FieldType = F;
	};
}


#define DEFINE_REFLECTION_PRIMITIVE(type) \
	template<> \
	const ::tako::Reflection::PrimitiveInformation* ::tako::Reflection::GetPrimitiveInformation<type>() \
	{ \
		static const ::tako::Reflection::PrimitiveInformation info(#type, sizeof(type)); \
		return &info; \
	}

DEFINE_REFLECTION_PRIMITIVE(int)
DEFINE_REFLECTION_PRIMITIVE(float)
DEFINE_REFLECTION_PRIMITIVE(bool)
DEFINE_REFLECTION_PRIMITIVE(unsigned char)
DEFINE_REFLECTION_PRIMITIVE(tako::U16)
DEFINE_REFLECTION_PRIMITIVE(tako::U64)
DEFINE_REFLECTION_PRIMITIVE(std::string) //TODO: handle separately?
