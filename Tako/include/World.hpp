#pragma once
#include "Entity.hpp"
#include "Archetype.hpp"
#include <unordered_map>
#include <functional>

namespace tako
{
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
}
