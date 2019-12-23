#pragma once
#include "Entity.hpp"
#include "Archetype.hpp"
#include <unordered_map>

namespace tako
{
	
	class World
	{
	public:
		World();

		Entity Create();
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
			U32 ent = m_nextId++;
			arch.AddEntity(ent);
			return ent;
		}

		void Delete();
	private:
		U32 m_nextId = 0;
		std::unordered_map<U64, Archetype> m_archetypes;
	};
}
