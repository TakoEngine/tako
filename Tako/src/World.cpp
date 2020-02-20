#include "World.hpp"

namespace tako
{
	World::World()
	{
		m_archetypes.insert({0, Archetype::Create<>()});
	}

	EntityHandle World::Create()
	{
		Entity ent = m_nextId++;
		return m_archetypes[0].AddEntity(ent);
	}

	void World::Delete(EntityHandle handle)
	{
	    handle.archeType->DeleteEntityFromChunk(*handle.chunk,handle.id, handle.indexChunk);
	}
}
