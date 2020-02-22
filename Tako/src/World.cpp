#include "World.hpp"

namespace tako
{
	World::World()
	{
		m_archetypes.insert({0, Archetype::Create<>()});
	}

	Entity World::Create()
	{
	    return CreateEntityInArchetype(m_archetypes[0]);
	}

	void World::Delete(Entity entity)
	{
	    auto& handle = m_entities[entity];
	    ASSERT(handle.id == entity);
	    auto swapped = handle.archeType->DeleteEntityFromChunk(*handle.chunk, entity, handle.indexChunk);
	    if (swapped)
        {
	        m_entities[swapped.value()].indexChunk = handle.indexChunk;
        }
	    std::swap(m_nextDeleted, handle.id);

	    m_deletedCount++;
	}

    Entity World::CreateEntityInArchetype(Archetype& arch)
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
}
