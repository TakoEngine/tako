#pragma once
#include <tuple>
#include <array>

// foreach macro: https://stackoverflow.com/questions/1872220/is-it-possible-to-iterate-over-arguments-in-variadic-macros
#define FE_0(WHAT)
#define FE_1(WHAT, X) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X),EXPAND(FE_1(WHAT, __VA_ARGS__))
#define FE_3(WHAT, X, ...) WHAT(X),EXPAND(FE_2(WHAT, __VA_ARGS__))
#define FE_4(WHAT, X, ...) WHAT(X),EXPAND(FE_3(WHAT, __VA_ARGS__))
#define FE_5(WHAT, X, ...) WHAT(X),EXPAND(FE_4(WHAT, __VA_ARGS__))
#define FE_6(WHAT, X, ...) WHAT(X),EXPAND(FE_5(WHAT, __VA_ARGS__))
#define FE_7(WHAT, X, ...) WHAT(X),EXPAND(FE_6(WHAT, __VA_ARGS__))

#define EXPAND( x ) x
#define GET_MACRO(_0,_1,_2,_3,_4,_5,_6,_7,NAME,...) NAME
#define FOR_EACH(action,...) \
  EXPAND(GET_MACRO(_0,__VA_ARGS__,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0)(action,__VA_ARGS__))

#define REFLECT_GET_FIELD_TYPE(name) decltype(name)

#define REFLECT_MEMBER_PTR(name) (::tako::Reflection::MemberPtr{#name,&SelfType::name})

#define REFLECT_GENERATE_TUPLE(...) using ReflectionTypeList = std::tuple<FOR_EACH(REFLECT_GET_FIELD_TYPE, __VA_ARGS__)>;
#define REFLECT_GENERATE_OFFSET_PTRS(...) static constexpr auto ReflectionMemberPtrs = std::make_tuple(FOR_EACH(REFLECT_MEMBER_PTR, __VA_ARGS__));

#define REFLECT_FIELD(name) {#name, offsetof(SelfType, name), ::tako::Reflection::Resolver::Get<decltype(SelfType::name)>()}

#define REFLECT(type,...) \
	using SelfType = type; \
	friend struct ::tako::Reflection::Resolver; \
	REFLECT_GENERATE_TUPLE(__VA_ARGS__) \
	REFLECT_GENERATE_OFFSET_PTRS(__VA_ARGS__) \
	static void InitReflection(::tako::Reflection::StructInformation* info) \
	{ \
		info->name = #type; \
		info->size = sizeof(SelfType); \
		info->constr = &::tako::Reflection::InitDefault<SelfType>; \
		info->fields = \
		{ \
			FOR_EACH(REFLECT_FIELD,__VA_ARGS__) \
		}; \
	} \
	static inline const ::tako::Reflection::StructInformation Reflection = {InitReflection};

#define REFLECT_CASE(name) ::tako::Reflection::EnumInformation::EnumCase{ #name, static_cast<std::size_t>(name) }

#define REFLECT_ENUM(TYPE, NAME, ...) \
	export template<> \
	const ::tako::Reflection::EnumInformation* ::tako::Reflection::GetEnumInformation<TYPE>() \
	{ \
		static auto InitReflection = [](::tako::Reflection::EnumInformation* info) -> void \
		{ \
			info->name = #NAME; \
			info->size = sizeof(TYPE); \
			info->convertUnderlying = [](const void* data) \
			{ \
				return static_cast<std::size_t>(*reinterpret_cast<const TYPE*>(data)); \
			}; \
			info->assignUnderlying = [](size_t data, void* target) \
			{ \
				*reinterpret_cast<TYPE*>(target) = static_cast<TYPE>(data); \
			}; \
			using enum TYPE; \
			static std::array cases = \
			{ \
				FOR_EACH(REFLECT_CASE, __VA_ARGS__) \
			}; \
			info->cases = cases; \
		}; \
		static ::tako::Reflection::EnumInformation info(InitReflection); \
		return &info; \
	}
