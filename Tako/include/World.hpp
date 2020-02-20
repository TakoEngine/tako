#pragma once
#include "Entity.hpp"
#include "Archetype.hpp"
#include <unordered_map>
#include <functional>
#include <tuple>

namespace tako
{
    namespace {

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


    }
	template<typename... Cs>
	class ComponentIterator;

	class World
	{
	public:
		World();

		EntityHandle Create();
		template<typename... Cs, typename = std::enable_if<(sizeof...(Cs) > 0)>>
		EntityHandle Create()
		{
			auto hash = GetArchetypeHash<Cs...>();
			auto iter = m_archetypes.find(hash);
			if (iter == m_archetypes.end())
			{
				m_archetypes[hash] = Archetype::Create<Cs...>();
			}
			Archetype& arch = m_archetypes[hash];
			U32 ent = m_nextId++;
			return arch.AddEntity(ent);
		}

		template<typename T>
		T& GetComponent(EntityHandle handle)
		{
			return handle.archeType->GetComponent<T>(*handle.chunk, handle.indexChunk);
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
            std::array<U8, sizeof...(Cs)> componentID = { { ComponentIDGenerator::GetID<Cs>()... } };
            auto indexSequence = std::index_sequence_for<Cs...>{};
            auto hash = GetArchetypeHash<Cs...>();
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
                        std::tuple<Cs*...> comps = TupleHelper<Cs...>::GetComponentArrays(arch, chunk, componentID, indexSequence);
                        auto arraySize = chunk.header.last;
                        for (int i = 0; i < arraySize; ++i)
                        {
                            TupleHelper<Cs...>::CallbackTuple(comps, i, callback, indexSequence);
                        }
                    }
                }
            }
        }

		void Delete(EntityHandle handle);
	private:
		U32 m_nextId = 0;
		std::unordered_map<U64, Archetype> m_archetypes;
	};

	template<typename... Cs>
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
