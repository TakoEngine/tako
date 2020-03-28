#pragma once
#include "NumberTypes.hpp"
#include "Entity.hpp"
#include "Utility.hpp"
#include <memory>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <optional>

namespace tako
{


    class ComponentIDGenerator
    {
        static U8 Identifier(std::size_t size)
        {
            static U8 value = 0;
            auto id = value++;
            m_componentSizes[id] = size;
            return id;
        }

        static std::map<U8, std::size_t> m_componentSizes;
    public:
        template<typename C>
        static U8 GetID()
        {
            static const U8 value = Identifier(sizeof(C));
            return value;
        }

        static std::size_t GetComponentSize(U8 id)
        {
            auto size = m_componentSizes.at(id);
            return size;
        }
    };



    namespace
    {
		template<typename C, typename... Cs>
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

	struct ChunkHeader
	{
		U16 last = 0;
	};

	struct Chunk
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

	struct Archetype;

	class EntityHandle
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

	struct Archetype
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

		Entity* GetEntityArray(Chunk& chunk)
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
		auto GetArray(Chunk& chunk)
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
}
