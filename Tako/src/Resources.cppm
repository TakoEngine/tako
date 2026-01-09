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
export import Tako.VFS;


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

struct CacheKey
{
	tako::U64 handle;
	TypeID type;

	bool operator==(const CacheKey& b) const
	{
		return handle == b.handle && type == b.type;
	}
};

template<>
struct std::hash<CacheKey>
{
	std::size_t operator()(const CacheKey& cacheKey) const
	{
		std::hash<tako::U64> hhash;
		std::hash<TypeID> thash;

		return hhash(cacheKey.handle) ^ (thash(cacheKey.type) << 1);
	}
};

namespace tako
{
	class ResourceLoaderFuncs
	{
		using LoadMemberPtr = U64(ResourceLoaderFuncs::*)(VFS*, const StringView);
		using ReleaseMemberPtr = void(ResourceLoaderFuncs::*)(U64);
		using ReloadMemberPtr = void(ResourceLoaderFuncs::*)(U64, VFS*, const StringView);
	public:
		ResourceLoaderFuncs()
		{
			m_loader = nullptr;
			m_memberLoadFunc = nullptr;
			m_loadInvoker = nullptr;
		}

		template<typename T, Handle R>
		ResourceLoaderFuncs(
			T* obj,
			R (T::*memFn)(VFS* vfs, const StringView),
			void (T::*releaseFn)(R),
			void(T::*reloadFn)(R, VFS* vfs, const StringView) = nullptr
		)
		{
			m_loader = obj;
			m_memberLoadFunc = reinterpret_cast<LoadMemberPtr>(memFn);
			m_loadInvoker = [](void* loader, LoadMemberPtr memberFunc, VFS* vfs, const StringView path) -> U64
			{
				T* o = static_cast<T*>(loader);
				auto m = reinterpret_cast<decltype(memFn)>(memberFunc);
				auto r = (o->*m)(vfs, path);
				return std::bit_cast<U64>(r);
			};
			m_memberReleaseFunc = reinterpret_cast<ReleaseMemberPtr>(releaseFn);
			m_releaseInvoker = [](void* loader, ReleaseMemberPtr memberFunc, U64 handle) -> void
			{
				T* o = static_cast<T*>(loader);
				auto m = reinterpret_cast<decltype(releaseFn)>(memberFunc);
				(o->*m)(std::bit_cast<R>(handle));
			};
			m_memberReloadFunc = reinterpret_cast<ReloadMemberPtr>(reloadFn);
			if (m_memberReloadFunc)
			{
				m_reloadInvoker = [](void* loader, ReloadMemberPtr memberFunc, U64 handle, VFS* vfs, const StringView path) -> void
				{
					T* o = static_cast<T*>(loader);
					auto m = reinterpret_cast<decltype(reloadFn)>(memberFunc);
					(o->*m)(std::bit_cast<R>(handle), vfs, path);
				};
			}
		}

		U64 Load(VFS* vfs, const StringView path)
		{
			return m_loadInvoker(m_loader, m_memberLoadFunc, vfs, path);
		}

		void Release(U64 handle)
		{
			m_releaseInvoker(m_loader, m_memberReleaseFunc, handle);
		}

		void Reload(U64 handle, VFS* vfs, const StringView path)
		{
			ASSERT(m_memberReloadFunc);
			m_reloadInvoker(m_loader, m_memberReloadFunc, handle, vfs, path);
		}

		bool CanReload() const
		{
			return m_memberReloadFunc;
		}

	private:
		void* m_loader;
		LoadMemberPtr m_memberLoadFunc;
		U64(*m_loadInvoker)(void*, LoadMemberPtr, VFS*, const StringView);
		ReleaseMemberPtr m_memberReleaseFunc;
		void(*m_releaseInvoker)(void*, ReleaseMemberPtr, U64);
		ReloadMemberPtr m_memberReloadFunc;
		void(*m_reloadInvoker)(void*, ReloadMemberPtr, U64, VFS*, const StringView) = nullptr;
	};

	using LoaderFunc = std::function<U64(StringView)>;

	export class Resources
	{
	public:
		Resources(VFS* vfs)
		{
			m_vfs = vfs;
		}

		template<Handle Handle, typename L>
		void RegisterLoader(
			L* loader,
			Handle(L::*loadFunc)(VFS* vfs, const StringView),
			void(L::*releaseFunc)(Handle),
			void(L::*reloadFunc)(Handle, VFS* vfs, const StringView) = nullptr
		)
		{
			ASSERT(loadFunc);
			ASSERT(releaseFunc);
			auto id = GetTypeID<Handle>();
			m_loaders[id] = ResourceLoaderFuncs(loader, loadFunc, releaseFunc, reloadFunc);
		}

		template<Handle Handle>
		Handle Load(StringView path)
		{
			auto typeID = GetTypeID<Handle>();
			{
				auto it = m_pathCache.find(path.ToStringView());
				if (it != m_pathCache.end())
				{
					auto& entry = it->second;
					ASSERT(typeID == entry.type);
					entry.refCount++;
					return std::bit_cast<Handle>(entry.handle);
				}
			}

			auto loader = m_loaders.find(typeID);
			ASSERT(loader != m_loaders.end());
			CacheEntry entry;
			entry.type = typeID;
			entry.handle = loader->second.Load(m_vfs, path);
			entry.refCount = 1;
			auto [inserted, _] = m_pathCache.emplace(std::make_pair(std::string(path), entry));
			m_handleCache.emplace(std::make_pair(entry, std::string_view(inserted->first)));
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

			auto itHandle = m_handleCache.find({ handle, typeID });
			ASSERT(itHandle != m_handleCache.end());
			auto itEntry = m_pathCache.find(itHandle->second);
			ASSERT(itEntry != m_pathCache.end());
			itEntry->second.refCount--;
			if (itEntry->second.refCount == 0)
			{
				m_pathCache.erase(itEntry);
				m_handleCache.erase(itHandle);
			}
		}

		template<Handle Handle>
		void Reload(Handle resource)
		{
			auto typeID = GetTypeID<Handle>();
			auto loader = m_loaders.find(typeID);
			ASSERT(loader != m_loaders.end());
			if (!loader->second.CanReload())
			{
				return;
			}
			auto handle = std::bit_cast<U64>(resource);
			auto itHandle = m_handleCache.find({ handle, typeID });
			ASSERT(itHandle != m_handleCache.end());
			loader->second.Reload(handle, itHandle->second);
		}

		void Reload(const StringView path)
		{
			auto itEntry = m_pathCache.find(path.ToStringView());
			if (itEntry == m_pathCache.end())
			{
				return;
			}
			auto loader = m_loaders.find(itEntry->second.type);
			ASSERT(loader != m_loaders.end());
			if (!loader->second.CanReload())
			{
				return;
			}
			loader->second.Reload(itEntry->second.handle, m_vfs, path);
		}
	private:
		VFS* m_vfs;
		struct CacheEntry : public CacheKey
		{
			U32 refCount;
		};

		std::unordered_map<TypeID, ResourceLoaderFuncs> m_loaders;
		std::unordered_map<std::string, CacheEntry, string_hash, std::equal_to<>> m_pathCache;
		std::unordered_map<CacheKey, std::string_view> m_handleCache;
	};
}
