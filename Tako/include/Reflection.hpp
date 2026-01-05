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
#define FE_8(WHAT, X, ...) WHAT(X),EXPAND(FE_7(WHAT, __VA_ARGS__))
#define FE_9(WHAT, X, ...) WHAT(X),EXPAND(FE_8(WHAT, __VA_ARGS__))
#define FE_10(WHAT, X, ...) WHAT(X),EXPAND(FE_9(WHAT, __VA_ARGS__))
#define FE_11(WHAT, X, ...) WHAT(X),EXPAND(FE_10(WHAT, __VA_ARGS__))
#define FE_12(WHAT, X, ...) WHAT(X),EXPAND(FE_11(WHAT, __VA_ARGS__))
#define FE_13(WHAT, X, ...) WHAT(X),EXPAND(FE_12(WHAT, __VA_ARGS__))
#define FE_14(WHAT, X, ...) WHAT(X),EXPAND(FE_13(WHAT, __VA_ARGS__))
#define FE_15(WHAT, X, ...) WHAT(X),EXPAND(FE_14(WHAT, __VA_ARGS__))
#define FE_16(WHAT, X, ...) WHAT(X),EXPAND(FE_15(WHAT, __VA_ARGS__))
#define FE_17(WHAT, X, ...) WHAT(X),EXPAND(FE_16(WHAT, __VA_ARGS__))
#define FE_18(WHAT, X, ...) WHAT(X),EXPAND(FE_17(WHAT, __VA_ARGS__))
#define FE_19(WHAT, X, ...) WHAT(X),EXPAND(FE_18(WHAT, __VA_ARGS__))
#define FE_20(WHAT, X, ...) WHAT(X),EXPAND(FE_19(WHAT, __VA_ARGS__))

#define EXPAND( x ) x
#define GET_MACRO(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,NAME,...) NAME
#define FOR_EACH(action,...) \
  EXPAND(GET_MACRO(_0,__VA_ARGS__,FE_20,FE_19,FE_18,FE_17,FE_16,FE_15,FE_14,FE_13,FE_12,FE_11,FE_10,FE_9,FE_8,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0)(action,__VA_ARGS__))

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
		info->constr = &::tako::Reflection::InitDefault<SelfType>;             \
        info->ConstructNew = []() -> void* \
		{ \
			return new SelfType(); \
		}; \
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

#define REFLECT_POLY(type) \
    using SelfType = type; \
	static void InitReflection(::tako::Reflection::PolymorphicInformation* info) \
	{ \
		info->name = #type; \
		info->size = sizeof(SelfType); \
		info->GetDerivedInfo = [](const void* uptr) -> const ::tako::Reflection::TypeInformation* \
		{ \
			return static_cast<const std::unique_ptr<SelfType>*>(uptr)->get()->ReflectionDerivedInfo(); \
		}; \
		info->Reset = [](void* uptr, void* data) \
		{ \
			static_cast<std::unique_ptr<SelfType>*>(uptr)->reset(static_cast<SelfType*>(data)); \
		}; \
	} \
	static inline const ::tako::Reflection::PolymorphicInformation Reflection = {InitReflection}; \
	virtual const ::tako::Reflection::StructInformation* ReflectionDerivedInfo() const { return nullptr; };

#define REFLECT_CHILD(type, ...) \
	REFLECT(type, __VA_ARGS__) \
	const ::tako::Reflection::StructInformation* ReflectionDerivedInfo() const override \
	{ \
		return &SelfType::Reflection; \
	}
