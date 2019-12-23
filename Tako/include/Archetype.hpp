#pragma once
#include "NumberTypes.hpp"
#include "Entity.hpp"
#include "Utility.hpp"
#include <memory>
#include <array>
#include <vector>
#include <algorithm>

namespace tako
{
	namespace
	{
		class ComponentIDGenerator
		{
			static U8 Identifier()
			{
				static U8 value = 0;
				return value++;
			}
		public:
			template<typename>
			static U8 GetID()
			{
				static const U8 value = Identifier();
				return value;
			}
		};

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


	//struct ComponentLayout
	//{
	//	std::unordered_map<U8, U32> offset;
	//};

	template<class... Cs>
	U16 FillComponentTypeInfo(std::vector<ComponenTypeInfo>& infos)
	{
		if constexpr (sizeof...(Cs) == 0)
		{
			return (CHUNK_SIZE - sizeof(ChunkHeader)) / sizeof(Entity);
		}
		else
		{
			std::size_t elementSize = CalculateEntitySize<Cs...>();
			U16 capacity = (CHUNK_SIZE - sizeof(ChunkHeader)) / elementSize;
			std::size_t offset = sizeof(Entity) * capacity;
			auto compInfoArray = GetIDArray<Cs...>();
			for (auto info : compInfoArray)
			{
				ComponenTypeInfo compInfo;
				compInfo.id = info.id;
				compInfo.size = info.size;
				compInfo.offset = offset;
				infos.push_back(compInfo);
				LOG("Offset {} {}", info.id, offset);
				offset += info.size * capacity;
			}

			return capacity;
		}
	}

	struct Archetype
	{
		Archetype()
		{
			chunk = std::make_unique<Chunk>();
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

		int AddEntity(Entity entity)
		{
			return AddEntityToChunk(*chunk, entity);
		}

		U64 componentHash;
		//std::vector<std::unique_ptr<Chunk>> chunks;
		std::unique_ptr<Chunk> chunk;
		std::vector<ComponenTypeInfo> componentInfo;
		U16 chunkCapacity;
	private:

		int AddEntityToChunk(Chunk& chunk, Entity entity)
		{
			int index = -1;

			if (chunk.header.last < chunkCapacity)
			{
				index = chunk.header.last;
				chunk.header.last++;

				Entity* entities = reinterpret_cast<Entity*>(&chunk.data[0]);
				entities[index] = entity;
			}

			return index;
		}

		template<typename T>
		T& GetComponent(Chunk& chunk, U16 index)
		{
			ASSERT(index < chunkCapacity);
			ASSERT(index < chunk.header.last);
			U8 compID = ComponentIDGenerator::GetID<T>();
			auto info = std::find_if(componentInfo.begin(), componentInfo.end(), [&](c)
			{
				return c.id == compID;
			});

			ASSERT(info != componentInfo.end());

			T* arr = reinterpret_cast<T*>(&chunk.data[info->offset]);
			return arr[index]
		}
	};
}
