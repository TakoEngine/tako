#pragma once
#include "Entity.hpp"
#include "Archetype.hpp"
#include <unordered_map>
#include <functional>

namespace tako
{
	template<typename T>
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

		template<typename C>
		ComponentIterator<C> Iter()
		{
			return ComponentIterator<C>(m_archetypes.begin(), m_archetypes.end());
		}

		template<typename... Cs, typename Cb>
		void Iterate(Cb callback)
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
					for (int ch = 0; ch < pair.second.chunks.size(); ch++)
					{
						Chunk& chunk = *pair.second.chunks[ch];
						C* comps = (C*) pair.second.GetComponentArray(chunk, componentID);
						for (int i = 0; i < chunk.header.last; i++)
						{
							callback(comps[i]);
						}
					}
					

				}
			}
		}

		void Delete();
	private:
		U32 m_nextId = 0;
		std::unordered_map<U64, Archetype> m_archetypes;
	};

	template<typename C>
	class ComponentIterator
	{
	public:
		ComponentIterator(std::unordered_map<U64, Archetype>::const_iterator begin, std::unordered_map<U64, Archetype>::const_iterator end)
		{
			m_archetypesIter = begin;
			m_archetypesEnd = end;
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

			
			m_archetypesIter++;
			SetupArcheType();

			return *this;
		}
		class IteratorSentinel {};
		bool operator!=(IteratorSentinel)
		{
			return m_archetypesIter != m_archetypesEnd;
		}

		C& operator*() const
		{
			return m_componentArray[m_indexComponentArray];
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
		C* m_componentArray;
		std::unordered_map<U64, Archetype>::const_iterator m_archetypesIter;
		std::unordered_map<U64, Archetype>::const_iterator m_archetypesEnd;

		void SetupChunk()
		{
			auto componentID = ComponentIDGenerator::GetID<C>();
			auto& pair = *m_archetypesIter;
			Chunk& chunk = *pair.second.chunks[m_indexChunks];
			m_componentArray = (C*) pair.second.GetComponentArray(chunk, componentID);
			m_indexComponentArray = 0;
			m_componentArraySize = chunk.header.last;
			//TODO: what if array is empty?
		}

		void SetupArcheType()
		{
			auto hash = GetArchetypeHash<C>();
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
