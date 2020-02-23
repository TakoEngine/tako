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
	    RemoveEntityFromArchetype(handle);

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

    void World::RemoveEntityFromArchetype(EntityHandle handle)
    {
        auto swapped = handle.archeType->DeleteEntityFromChunk(*handle.chunk, handle.id, handle.indexChunk);
        if (swapped)
        {
            m_entities[swapped.value()].indexChunk = handle.indexChunk;
        }
    }

    void World::MoveEntityArchetype(EntityHandle handle, U64 targetHash)
    {
        auto iter = m_archetypes.find(targetHash);
        if (iter == m_archetypes.end())
        {
            //TODO: Create new Archetype
            LOG_ERR("Cant create new Archetype (NOT IMPLEMENTED)")
            return;
        }

        auto& targetArch = iter->second;
        auto targetHandle = targetArch.AddEntity(handle.id);
        targetArch.CopyComponentData(handle, *targetHandle.chunk, handle.id, targetHandle.indexChunk);
        RemoveEntityFromArchetype(handle);
        m_entities[handle.id] = targetHandle;
    }
}
