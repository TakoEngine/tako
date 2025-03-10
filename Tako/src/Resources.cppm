module;
#include "Utility.hpp"
#include <unordered_map>
#include <functional>
#include <bit>
#include <string>
export module Tako.Resources;

import Tako.StringView;
import Tako.NumberTypes;
import Tako.HandleVec;


using TypeID = size_t;

template<typename T>
TypeID GetTypeID()
{
	static char id;
	return reinterpret_cast<TypeID>(&id);
}

struct string_hash
{
	using hash_type = std::hash<std::string_view>;
	using is_transparent = void;

	std::size_t operator()(const char* str) const { return hash_type{}(str); }
	std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
	std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
};

namespace tako
{
	class ResourceLoaderFuncs
	{
		using LoadMemberPtr = U64(ResourceLoaderFuncs::*)(const StringView);
		using ReleaseMemberPtr = void(ResourceLoaderFuncs::*)(U64);
	public:
		ResourceLoaderFuncs()
		{
			m_loader = nullptr;
			m_memberLoadFunc = nullptr;
			m_loadInvoker = nullptr;
		}

		template<typename T, Handle R>
		ResourceLoaderFuncs(T* obj, R (T::*memFn)(const StringView), void (T::*releaseFn)(R))
		{
			m_loader = obj;
			m_memberLoadFunc = reinterpret_cast<LoadMemberPtr>(memFn);
			m_loadInvoker = [](void* loader, LoadMemberPtr memberFunc, const StringView path) -> U64
			{
				T* o = static_cast<T*>(loader);
				auto m = reinterpret_cast<decltype(memFn)>(memberFunc);
				auto r = (o->*m)(path);
				return std::bit_cast<U64>(r);
			};
			m_memberReleaseFunc = reinterpret_cast<ReleaseMemberPtr>(releaseFn);
			m_releaseInvoker = [](void* loader, ReleaseMemberPtr memberFunc, U64 handle) -> void
			{
				T* o = static_cast<T*>(loader);
				auto m = reinterpret_cast<decltype(releaseFn)>(memberFunc);
				(o->*m)(std::bit_cast<R>(handle));
			};
		}

		U64 Load(const StringView path)
		{
			return m_loadInvoker(m_loader, m_memberLoadFunc, path);
		}

		void Release(U64 handle)
		{
			m_releaseInvoker(m_loader, m_memberReleaseFunc, handle);
		}

	private:
		void* m_loader;
		LoadMemberPtr m_memberLoadFunc;
		U64(*m_loadInvoker)(void*, LoadMemberPtr, const StringView);
		ReleaseMemberPtr m_memberReleaseFunc;
		void(*m_releaseInvoker)(void*, ReleaseMemberPtr, U64);
	};

	using LoaderFunc = std::function<U64(StringView)>;

	export class Resources
	{
	public:
		Resources()
		{
		}

		template<Handle Handle, typename L>
		void RegisterLoader(L* loader, Handle(L::*loadFunc)(const StringView), void(L::*releaseFunc)(Handle))
		{
			auto id = GetTypeID<Handle>();
			m_loaders[id] = ResourceLoaderFuncs(loader, loadFunc, releaseFunc);
		}

		template<Handle Handle>
		Handle Load(StringView path)
		{
			auto typeID = GetTypeID<Handle>();
			{
				auto it = m_cache.find(path.ToStringView());
				if (it != m_cache.end())
				{
					auto& entry = it->second;
					ASSERT(typeID == entry.type);
					return std::bit_cast<Handle>(entry.handle);
				}
			}

			auto loader = m_loaders.find(typeID);
			ASSERT(loader != m_loaders.end());
			CacheEntry entry;
			entry.type = typeID;
			entry.handle = loader->second.Load(path);
			m_cache.emplace(std::make_pair(std::string(path), entry));
			return std::bit_cast<Handle>(entry.handle);
		}

		template<Handle Handle>
		void Release(Handle resource)
		{
			auto typeID = GetTypeID<Handle>();
			auto loader = m_loaders.find(typeID);
			ASSERT(loader != m_loaders.end());
			auto handle = std::bit_cast<U64>(resource);
			loader->second.Release(handle);

			//TODO: quite costly for many resources
			for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
			{
				if (it->second.type == typeID && it->second.handle == handle)
				{
					m_cache.erase(it);
					break;
				}
			}
		}
	private:
		struct CacheEntry
		{
			U64 handle;
			TypeID type;
		};

		std::unordered_map<TypeID, ResourceLoaderFuncs> m_loaders;
		std::unordered_map<std::string, CacheEntry, string_hash, std::equal_to<>> m_cache;
	};
}

