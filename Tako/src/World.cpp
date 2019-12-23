#include "World.hpp"

namespace tako
{
	World::World()
	{
		m_archetypes.insert({0, Archetype::Create<>()});
	}

	Entity World::Create()
	{
		Entity ent = m_nextId++;
		m_archetypes[0].AddEntity(ent);
		return ent;
	}

	void World::Delete()
	{
	}
}
