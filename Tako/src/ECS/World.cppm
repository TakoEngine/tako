module;
#include "NumberTypes.hpp"
#include "Entity.hpp"
#include "Utility.hpp"
#include <unordered_map>
#include <functional>
#include <tuple>
#include <array>
#include <utility>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <optional>
#include <ranges>

export module Tako.World;

namespace tako
{
// Archetype
	export class ComponentIDGenerator
	{
		static U8 Identifier(std::size_t size)
		{
			static U8 value = 0;
			auto id = value++;
			m_componentSizes[id] = size;
			return id;
		}

		inline static std::map<U8, std::size_t> m_componentSizes = {};
	public:
		template<typename C>
		static U8 GetID()
		{
			if constexpr (std::is_same_v<C, Entity>)
			{
				return 0;
			}
			static const U8 value = Identifier(sizeof(C));
			return value;
		}

		static std::size_t GetComponentSize(U8 id)
		{
			auto size = m_componentSizes.at(id);
			return size;
		}
	};

	export template<typename C, typename... Cs>
	U64 GetArchetypeHash()
	{
		U64 bit = 1 << static_cast<U64>(ComponentIDGenerator::GetID<C>());
		if constexpr (sizeof...(Cs) == 0)
		{
			return bit;
		}
		else
		{
			return bit | GetArchetypeHash<Cs...>();
		}
	}

	struct CompInfo
	{
		U8 id;
		std::size_t size;
	};

	template<std::size_t index, std::size_t size, typename C, typename... Cs>
	void FillIDArray(std::array<CompInfo, size>& arr)
	{
		CompInfo info;
		info.id = ComponentIDGenerator::GetID<C>();
		info.size = sizeof(C);
		arr[index] = info;
		if constexpr (sizeof...(Cs) > 0)
		{
			FillIDArray<index + 1, size, Cs...>(arr);
		}
	}

	template<typename... Cs>
	std::array<CompInfo, sizeof...(Cs)> GetIDArray()
	{
		std::array<CompInfo, sizeof...(Cs)> arr;
		FillIDArray<0, sizeof...(Cs), Cs...>(arr);
		return arr;
	}


	std::size_t CalculateEntitySize()
	{
		return sizeof(Entity);
	}

	template<typename C, typename... Cs>
	std::size_t CalculateEntitySize()
	{
		if constexpr (sizeof...(Cs) == 0)
		{
			return sizeof(C) + sizeof(Entity);
		}
		else
		{
			return sizeof(C) + CalculateEntitySize<Cs...>();
		}

	}

/*
	Chunk:
	Entity[]
	Comp1[]
	Comp2[]
	Comp3[]
	...
	CompN[]
*/
	constexpr std::size_t CHUNK_SIZE = 16 * 1024; //16kb

	export struct ChunkHeader
	{
		U16 last = 0;
	};

	export struct Chunk
	{
		ChunkHeader header;
		U8 data[CHUNK_SIZE - sizeof(ChunkHeader)];
	};

	struct ComponenTypeInfo
	{
		U8 id;
		std::size_t size;
		std::size_t offset;
	};

	U16 FillComponentTypeInfo(std::map<U8, ComponenTypeInfo>& infos, const CompInfo* compInfos, std::size_t infoCount, std::size_t elementSize)
	{
		if (infoCount == 0)
		{
			return (CHUNK_SIZE - sizeof(ChunkHeader)) / sizeof(Entity);
		}
		else
		{
			U16 capacity = (CHUNK_SIZE - sizeof(ChunkHeader)) / elementSize;
			std::size_t offset = sizeof(Entity) * capacity;
			for (int i = 0; i < infoCount; i++)
			{
				auto info = compInfos[i];
				ComponenTypeInfo compInfo;
				compInfo.id = info.id;
				compInfo.size = info.size;
				compInfo.offset = offset;
				infos.emplace(info.id, compInfo);
				//LOG("Offset {} {}", info.id, offset);
				offset += info.size * capacity;
			}

			return capacity;
		}
	}


	template<class... Cs>
	U16 FillComponentTypeInfo(std::map<U8, ComponenTypeInfo>& infos)
	{
		if constexpr (sizeof...(Cs) == 0)
		{
			return (CHUNK_SIZE - sizeof(ChunkHeader)) / sizeof(Entity);
		}
		else
		{
			auto compInfoArray = GetIDArray<Cs...>();
			return FillComponentTypeInfo(infos, compInfoArray.data(), compInfoArray.size(), CalculateEntitySize<Cs...>());
		}
	}

	U16 FillComponentTypeInfo(std::map<U8, ComponenTypeInfo>& infos, const CompInfo* compInfos, std::size_t infoCount)
	{
		std::size_t elementSize = sizeof(Entity);
		for (int i = 0; i < infoCount; i++)
		{
			elementSize += compInfos[i].size;
		}

		return FillComponentTypeInfo(infos, compInfos, infoCount, elementSize);
	}

	export struct Archetype;

	export class EntityHandle
	{
	public:
		Entity id;
	private:
		Archetype* archeType;
		Chunk* chunk;
		int indexChunk;
		friend class Archetype;
		friend class World;
	};

	export struct Archetype
	{
		U64 componentHash;
		std::vector<std::unique_ptr<Chunk>> chunks;
		int chunksFilled = 0;
		//std::vector<ComponenTypeInfo> componentInfo;
		std::map<U8, ComponenTypeInfo> componentInfo;
		U16 chunkCapacity;

		Archetype()
		{
			chunks.emplace_back(std::make_unique<Chunk>());
		}

		template<class... Cs>
		static Archetype Create()
		{
			U64 hash;
			if constexpr (sizeof...(Cs) == 0)
			{
				hash = 0;
			}
			else
			{
				hash = GetArchetypeHash<Cs...>();
			}

			Archetype arch;
			arch.componentHash = hash;
			arch.chunkCapacity = FillComponentTypeInfo<Cs...>(arch.componentInfo);
			return arch;
		}

		static Archetype Create(U64 hash)
		{
			std::array<CompInfo, 64> componentInfos;
			std::size_t componentInfoCount = 0;

			U64 tempHash = hash;
			for (U64 i = 0; tempHash > 0; i++)
			{
				if (tempHash & (static_cast<U64>(1) << i))
				{
					componentInfos[componentInfoCount].id = i;
					componentInfos[componentInfoCount].size = ComponentIDGenerator::GetComponentSize(i);
					componentInfoCount++;
					tempHash &= ~(static_cast<U64>(1) << i);
				}
			}

			Archetype arch;
			arch.componentHash = hash;
			arch.chunkCapacity = FillComponentTypeInfo(arch.componentInfo, componentInfos.data(), componentInfoCount);
			return arch;
		}

		EntityHandle AddEntity(Entity entity)
		{
			int index;
			if (chunksFilled == chunks.size())
			{
				chunks.emplace_back(std::make_unique<Chunk>());
			}

			Chunk& chunk = *chunks[chunksFilled];
			index = AddEntityToChunk(chunk, entity);

			if (chunk.header.last >= chunkCapacity)
			{
				chunksFilled++;
			}
			EntityHandle handle;
			handle.id = entity;
			handle.archeType = this;
			handle.chunk = &chunk;
			handle.indexChunk = index;
			return handle;
		}

		void CopyComponentData(EntityHandle src, Chunk& chunk, Entity entity, int index)
		{
			ASSERT(GetEntityArray(chunk)[index] == entity);

			for (auto [id, info] : componentInfo)
			{
				//auto arr = ch
				auto& srcComponentInfo = src.archeType->componentInfo;
				if (srcComponentInfo.find(id) != srcComponentInfo.end())
				{
					U8* srcArray = &src.chunk->data[srcComponentInfo[id].offset];
					U8* compArray = &chunk.data[info.offset];
					std::memcpy(compArray + info.size * index, srcArray + info.size * src.indexChunk, info.size);
				}

			}
		}


		int AddEntityToChunk(Chunk& chunk, Entity entity)
		{
			int index = -1;

			if (chunk.header.last < chunkCapacity)
			{
				index = chunk.header.last;
				chunk.header.last++;

				Entity* entities = GetEntityArray(chunk);
				entities[index] = entity;
			}

			return index;
		}

		std::optional<Entity> DeleteEntityFromChunk(Chunk& chunk, Entity entity, int index)
		{
			ASSERT(GetEntityArray(chunk)[index] == entity);
			bool wasFull = chunk.header.last >= chunkCapacity;
			std::optional<Entity> swapped;

			chunk.header.last--;
			if (index < chunk.header.last)
			{
				Entity* entities = GetEntityArray(chunk);
				swapped = entities[index] = entities[chunk.header.last];
				for (auto [id, info] : componentInfo)
				{
					//auto arr = ch
					U8* compArray = &chunk.data[info.offset];
					std::memcpy(compArray + info.size * index, compArray + info.size * chunk.header.last, info.size);
				}
			}


			if (wasFull)
			{
				chunksFilled--;
				auto ptr = &chunk;
				for (int i = 0; i < chunks.size(); i++)
				{
					if (ptr == chunks[i].get())
					{
						if (i < chunksFilled)
						{
							std::swap(chunks[i], chunks[chunksFilled]);
						}
						break;
					}
				}
			}

			return swapped;
		}

		Entity* GetEntityArray(Chunk& chunk) const
		{
			return reinterpret_cast<Entity*>(&chunk.data[0]);
		}

		void* GetComponentArray(Chunk& chunk, U8 componentID) const
		{
			auto info = componentInfo.at(componentID);
			//ASSERT(info != componentInfo.end());

			return &chunk.data[info.offset];
		}

		template<typename T>
		auto GetArray(Chunk& chunk) const
		{
			if constexpr (std::is_same<T, Entity>::value)
			{
				return GetEntityArray(chunk);
			}
			else
			{
				return GetComponentArray(chunk, ComponentIDGenerator::GetID<T>());
			}
		}

		template<typename T>
		T& GetComponent(Chunk& chunk, U16 index)
		{
			ASSERT(index < chunkCapacity);
			ASSERT(index < chunk.header.last);
			U8 compID = ComponentIDGenerator::GetID<T>();
			auto info = componentInfo.at(compID);
			//ASSERT(info != componentInfo.end());

			T* arr = reinterpret_cast<T*>(&chunk.data[info.offset]);
			return arr[index];
		}

		template<typename T>
		bool HasComponent(Chunk& chunk, U16 index)
		{
			ASSERT(index < chunkCapacity);
			ASSERT(index < chunk.header.last);
			U8 compID = ComponentIDGenerator::GetID<T>();
			auto info = componentInfo.find(compID);
			return info != componentInfo.end();
		}
	};

//World
	template <class... Args>
	struct type_list
	{
		template <std::size_t N>
		using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
	};

	template<typename... Cs>
	class TupleHelper
	{
	public:
		template<std::size_t... I>
		static inline auto GetComponentArrays(const Archetype& arch, Chunk& chunk, const std::array<U8,sizeof...(Cs)>& componentID, std::index_sequence<I...>)
		{
			return std::make_tuple((static_cast<typename type_list<Cs*...>::template type<I>>(arch.GetComponentArray(chunk, std::get<I>(componentID))))...);
		}

		template<std::size_t... I>
		static inline std::tuple<Cs&...> CreateTuple(const std::tuple<Cs*...>& componentArray, int index, std::index_sequence<I...>)
		{
			return std::make_tuple(std::ref(std::get<I>(componentArray)[index])...);
		}

		template<typename Cb, std::size_t... I>
		static inline void CallbackTuple(const std::tuple<Cs*...>& componentArray, int index, Cb callback, std::index_sequence<I...>)
		{
			callback(std::ref(std::get<I>(componentArray)[index])...);
		}
	};

	template<typename... Cs>
	auto GetCompIDArray()
	{
		return std::array<U8, sizeof...(Cs)>({ComponentIDGenerator::GetID<Cs>()... });
	}

	template<typename C, typename... Cs>
	class EntityTupleHelper
	{
	public:
		constexpr static auto HasEntity = std::is_same_v<C, Entity>;
		constexpr static auto compCount = HasEntity ? sizeof...(Cs) : sizeof...(Cs) + 1;
		constexpr static auto FullSequence = std::index_sequence_for<C, Cs...>();
		constexpr static auto TailSequence = std::index_sequence_for<Cs...>();
		//constexpr static auto IndexSequence = HasEntity ? TailSequence : FullSequence;
		using types = std::conditional<HasEntity, type_list<C, Cs...>, type_list<Cs...>>;
		using Tuple = std::conditional<HasEntity, type_list<C, Cs*...>, type_list<C*, Cs*...>>;

		constexpr static auto GetHash()
		{
			if constexpr (HasEntity)
			{
				if constexpr (sizeof...(Cs) == 0)
				{
					return 0;
				}
				else
				{
					return GetArchetypeHash<Cs...>();
				}
			}
			else
			{
				return GetArchetypeHash<C, Cs...>();
			}
		}

		static auto GetIDArray()
		{
			if constexpr (HasEntity)
			{
				return GetCompIDArray<Cs...>();
			}
			else
			{
				return GetCompIDArray<C, Cs...>();
			}
		}

		static inline const std::tuple<C*, Cs*...> GetComponentArrays(const Archetype& arch, Chunk& chunk, const std::array<U8, compCount>& componentID)
		{
			if constexpr (HasEntity)
			{
				return GetComponentArraysSequence(arch, chunk, componentID, TailSequence);
			}
			else
			{
				return GetComponentArraysSequence(arch, chunk, componentID, FullSequence);
			}
		}

		template<std::size_t... I>
		static inline const std::tuple<C*, Cs*...> GetComponentArraysSequence(const Archetype& arch, Chunk& chunk, const std::array <U8, compCount>& componentID, std::index_sequence<I...>)
		{
			if constexpr (HasEntity)
			{
				return std::make_tuple(arch.GetEntityArray(chunk), (static_cast<typename type_list<Cs*...>::template type<I>>(arch.GetComponentArray(chunk, std::get<I>(componentID))))...);
			}
			else
			{
				return std::make_tuple((static_cast<typename type_list<C*, Cs*...>::template type<I>>(arch.GetComponentArray(chunk, std::get<I>(componentID))))...);
			}

		}

		template<typename Cb>
		static inline void CallbackTuple(const std::tuple<C*, Cs*...>& componentArray, int index, Cb callback)
		{
			return CallbackTupleSequence(componentArray, index, callback, FullSequence);
		}

		static inline auto CreateTuple(const std::tuple<C*, Cs*...>& componentArray, int index)
		{
			return CreateTupleSequence(componentArray, index, FullSequence);
		}

		template<std::size_t I>
		static inline auto IndexCompArray(const std::tuple<C*, Cs*...>& componentArray, int index)
		{
			if constexpr (HasEntity && I == 0)
			{
				return std::get<I>(componentArray)[index];
			}
			else
			{
				return std::ref(std::get<I>(componentArray)[index]);
			}
		}

		template<typename Cb, std::size_t... I>
		static inline void CallbackTupleSequence(const std::tuple<C*, Cs*...>& componentArray, int index, Cb callback, std::index_sequence<I...>)
		{
			callback(IndexCompArray<I>(componentArray, index)...);
		}

		template<std::size_t... I>
		static inline auto CreateTupleSequence(const std::tuple<C*, Cs*...>& componentArray, int index, std::index_sequence<I...>)
		{
			return std::make_tuple(IndexCompArray<I>(componentArray, index)...);
		}
	};


	export template<typename... Cs>
	class ComponentIterator;

	export class World
	{
	public:
		World()
		{
			m_archetypes.insert({ 0, Archetype::Create<>() });
		}

		Entity Create()
		{
			return CreateEntityInArchetype(m_archetypes[0]);
		}

		template<typename... Cs, typename = std::enable_if<(sizeof...(Cs) > 0)>>
		Entity Create()
		{
			auto hash = GetArchetypeHash<Cs...>();
			auto iter = m_archetypes.find(hash);
			if (iter == m_archetypes.end())
			{
				m_archetypes[hash] = Archetype::Create<Cs...>();
			}
			Archetype& arch = m_archetypes[hash];
			return CreateEntityInArchetype(arch);
		}

		template<typename... Cs, typename = std::enable_if<(sizeof...(Cs) > 0)>>
		Entity Create(Cs&&... comps)
		{
			auto ent = Create<Cs...>();
			auto handle = m_entities[ent];
			(handle.archeType->template GetComponent<Cs>(*handle.chunk, handle.indexChunk).operator=(std::move(comps)), ...);
			return ent;
		}

		template<typename T>
		T& GetComponent(Entity entity)
		{
			auto handle = m_entities[entity];
			return handle.archeType->GetComponent<T>(*handle.chunk, handle.indexChunk);
		}

		template<typename T>
		bool HasComponent(Entity entity)
		{
			auto handle = m_entities[entity];
			return handle.archeType->HasComponent<T>(*handle.chunk, handle.indexChunk);
		}

		template<typename T>
		void AddComponent(Entity entity)
		{
			auto compID = ComponentIDGenerator::GetID<T>();
			auto handle = m_entities[entity];
			auto hash = handle.archeType->componentHash;
			auto newHash = hash | (1 << static_cast<U64>(compID));
			if (hash == newHash)
			{
				LOG("same hash");
				return;
			}

			MoveEntityArchetype(handle, newHash);
		}

		template<typename T>
		void AddComponent(Entity entity, const T& component)
		{
			AddComponent<T>(entity);
			GetComponent<T>(entity) = component;
		}

		template<typename T>
		void RemoveComponent(Entity entity)
		{
			auto compID = ComponentIDGenerator::GetID<T>();
			auto handle = m_entities[entity];
			auto hash = handle.archeType->componentHash;
			auto newHash = hash & (~(1 << static_cast<U64>(compID)));
			if (hash == newHash)
			{
				LOG("same hash");
				return;
			}

			MoveEntityArchetype(handle, newHash);
		}

		template<typename... Cs>
		ComponentIterator<Cs...> Iter()
		{
			return ComponentIterator<Cs...>(m_archetypes.begin(), m_archetypes.end());
		}

		template<typename... Cs, typename Cb>
		void IterateHandle(Cb callback)
		{
			auto hash = GetArchetypeHash<Cs...>();
			EntityHandle handle;
			for (auto& pair : m_archetypes)
			{
				U64 archHash = pair.first;
				if ((archHash & hash) == hash)
				{
					handle.archeType = &pair.second;
					for (int ch = 0; ch < pair.second.chunks.size(); ch++)
					{
						auto& chunk = *pair.second.chunks[ch];
						Entity* entities = handle.archeType->GetArray<Entity>(chunk);
						handle.chunk = &chunk;
						for (int i = 0; i < chunk.header.last; i++)
						{
							handle.id = entities[i];
							handle.indexChunk = i;
							callback(handle);
						}
					}
				}
			}
		}

		template<typename C, typename Cb>
		void IterateComp(Cb callback)
		{
			auto componentID = ComponentIDGenerator::GetID<C>();
			auto hash = GetArchetypeHash<C>();
			for (auto& pair : m_archetypes)
			{
				U64 archHash = pair.first;
				if ((archHash & hash) == hash)
				{
					auto& arch = pair.second;
					auto chunkSize = arch.chunks.size();
					for (int ch = 0; ch < chunkSize; ++ch)
					{
						Chunk& chunk = *arch.chunks[ch];
						C* comps = (C*) arch.GetComponentArray(chunk, componentID);
						auto arraySize = chunk.header.last;
						for (int i = 0; i < arraySize; ++i)
						{
							callback(comps[i]);
						}
					}


				}
			}
		}

		template<typename... Cs, typename Cb, typename=void>
		void IterateComps(Cb callback)
		{
			auto componentID = EntityTupleHelper<Cs...>::GetIDArray();
			auto hash = EntityTupleHelper<Cs...>::GetHash();
			for (auto& pair : m_archetypes)
			{
				U64 archHash = pair.first;
				if ((archHash & hash) == hash)
				{
					auto& arch = pair.second;
					auto chunkSize = arch.chunks.size();
					for (int ch = 0; ch < chunkSize; ++ch)
					{
						Chunk& chunk = *arch.chunks[ch];
						auto comps = EntityTupleHelper<Cs...>::GetComponentArrays(arch, chunk, componentID);
						auto arraySize = chunk.header.last;
						for (int i = 0; i < arraySize; ++i)
						{
							EntityTupleHelper<Cs...>::CallbackTuple(comps, i, callback);
						}
					}
				}
			}
		}

		template<typename... Cs>
		auto Iterate()
		{
			return m_archetypes
			| std::views::filter([hash = EntityTupleHelper<Cs...>::GetHash()](auto& pair)
			{
				return (pair.first & hash) == hash;
			})
			| std::views::transform([componentID = EntityTupleHelper<Cs...>::GetIDArray()](auto& pair)
			{
				auto& arch = pair.second;
				return arch.chunks | std::views::transform([&](auto& chunk)
				{
					auto comps = EntityTupleHelper<Cs...>::GetComponentArrays(arch, *chunk, componentID);
					auto chunkIndices = std::views::iota(0) | std::views::take(chunk->header.last);
					return chunkIndices | std::views::transform([comps = std::move(comps)](auto i)
					{
						return EntityTupleHelper<Cs...>::CreateTuple(comps, i);
					});
				}) | std::views::join;
			}) | std::views::join;
		}

		void Delete(Entity entity)
		{
			auto& handle = m_entities[entity];
			ASSERT(handle.id == entity);
			RemoveEntityFromArchetype(handle);

			std::swap(m_nextDeleted, handle.id);
			m_deletedCount++;
		}

		void Reset()
		{
			m_entities.clear();
			m_nextDeleted = 0;
			m_deletedCount = 0;
			m_archetypes.clear();
		}
	private:
		std::vector<EntityHandle> m_entities;
		U32 m_nextDeleted = 0;
		std::size_t m_deletedCount = 0;
		std::unordered_map<U64, Archetype> m_archetypes;

		Entity CreateEntityInArchetype(Archetype& arch)
		{
			Entity ent;
			bool newID = m_deletedCount == 0;

			if (newID)
			{
				ent = m_entities.size();
			}
			else
			{
				ent = m_nextDeleted;
				m_nextDeleted = m_entities[ent].id;
				m_deletedCount--;
			}

			auto handle = arch.AddEntity(ent);
			if (newID)
			{
				m_entities.push_back(handle);
			}
			else
			{
				m_entities[ent] = handle;
			}

			return ent;
		}

		void RemoveEntityFromArchetype(EntityHandle handle)
		{
			auto swapped = handle.archeType->DeleteEntityFromChunk(*handle.chunk, handle.id, handle.indexChunk);
			if (swapped)
			{
				m_entities[swapped.value()].indexChunk = handle.indexChunk;
			}
		}

		void MoveEntityArchetype(EntityHandle handle, U64 targetHash)
		{
			auto iter = m_archetypes.find(targetHash);
			Archetype* targetArch;
			if (iter == m_archetypes.end())
			{
				m_archetypes[targetHash] = Archetype::Create(targetHash);
				targetArch = &m_archetypes[targetHash];
			}
			else
			{
				targetArch = &iter->second;
			}

			auto targetHandle = targetArch->AddEntity(handle.id);
			targetArch->CopyComponentData(handle, *targetHandle.chunk, handle.id, targetHandle.indexChunk);
			RemoveEntityFromArchetype(handle);
			m_entities[handle.id] = targetHandle;
		}
	};

	export template<typename... Cs>
	class ComponentIterator
	{
	public:
		ComponentIterator(std::unordered_map<U64, Archetype>::const_iterator begin, std::unordered_map<U64, Archetype>::const_iterator end)
		{
			m_archetypesIter = begin;
			m_archetypesEnd = end;
			hash = GetArchetypeHash<Cs...>();
			componentID = { {ComponentIDGenerator::GetID<Cs>()...} }; //ComponentIDGenerator::GetID<C>();
			SetupArcheType();
		}

		ComponentIterator& operator++()
		{
			if (m_indexComponentArray + 1 < m_componentArraySize)
			{
				++m_indexComponentArray;
				return *this;
			}

			if (m_indexChunks + 1 < m_chunksSize)
			{
				++m_indexChunks;
				SetupChunk();
				return *this;
			}


			++m_archetypesIter;
			SetupArcheType();

			return *this;
		}
		class IteratorSentinel {};
		bool operator!=(IteratorSentinel)
		{
			return m_archetypesIter != m_archetypesEnd;
		}

		std::tuple<Cs&...> operator*() const
		{
			return CreateTuple(std::index_sequence_for<Cs*...>{});
		}

		template<std::size_t... I>
		std::tuple<Cs&...> CreateTuple(std::index_sequence<I...>) const
		{
			return std::make_tuple(std::ref(std::get<I>(m_componentArray)[m_indexComponentArray])...);
		}

		ComponentIterator begin() const
		{
			return *this;
		}
		IteratorSentinel end() const
		{
			return {};
		}



	private:
		int m_indexComponentArray;
		int m_componentArraySize;
		int m_indexChunks;
		int m_chunksSize;
		std::tuple<Cs*...> m_componentArray;
		std::unordered_map<U64, Archetype>::const_iterator m_archetypesIter;
		std::unordered_map<U64, Archetype>::const_iterator m_archetypesEnd;
		U64 hash;
		std::array<U8,sizeof...(Cs)> componentID;

		inline void SetupChunk()
		{
			auto& pair = *m_archetypesIter;
			Chunk& chunk = *pair.second.chunks[m_indexChunks];
			m_componentArray = GetComponentArrays(pair.second, chunk, std::index_sequence_for<Cs...>{});
			m_indexComponentArray = 0;
			m_componentArraySize = chunk.header.last;
			//TODO: what if array is empty?
		}

		template <class... Args>
		struct type_list
		{
			template <std::size_t N>
			using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
		};

		template<std::size_t... I>
		auto GetComponentArrays(const Archetype& arch, Chunk& chunk, std::index_sequence<I...>)
		{
			return std::make_tuple((static_cast<typename type_list<Cs*...>::template type<I>>(arch.GetComponentArray(chunk, std::get<I>(componentID))))...);
		}

		inline void SetupArcheType()
		{

			while (m_archetypesIter != m_archetypesEnd)
			{

				auto& pair = *m_archetypesIter;
				if ((pair.first & hash) != hash)
				{
					++m_archetypesIter;
					continue;
				}

				m_chunksSize = pair.second.chunks.size();
				m_indexChunks = 0;
				SetupChunk();
				break;
			}
		}
	};
}
