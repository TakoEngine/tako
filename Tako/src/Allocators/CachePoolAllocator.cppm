module;
#include "Utility.hpp"
#include "NumberTypes.hpp"
export module Tako.Allocators.CachePoolAllocator;

import Tako.Allocators.Allocator;

namespace tako::Allocators
{
	export class CachePoolAllocator final : public Allocator
	{
		struct Node
		{
			void* next = nullptr;
			size_t size;
		};
	public:

		~CachePoolAllocator()
		{
			while (m_freeList != nullptr)
			{
				Node* node = reinterpret_cast<Node*>(m_freeList);
				m_freeList = node->next;
				free(node);
			}
		}


		virtual void* Allocate(size_t size) override
		{
			ASSERT(size >= sizeof(Node));
			while (m_freeList != nullptr)
			{
				Node* node = reinterpret_cast<Node*>(m_freeList);
				m_freeList = node->next;
				if (node->size == size)
				{
					return node;
				}

				free(node);
			}

			return malloc(size);
		}

		virtual void Deallocate(void* p, size_t size) override
		{
			Node* node = reinterpret_cast<Node*>(p);
			node->next = m_freeList;
			node->size = size;

			m_freeList = node;
		}
	private:
		void* m_freeList = nullptr;
	};
}
