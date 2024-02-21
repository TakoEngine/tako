module;
#include "Utility.hpp"
#include "NumberTypes.hpp"
export module Tako.Allocators.PoolAllocator;

import Tako.Allocators.Allocator;

namespace tako::Allocators
{
	export class PoolAllocator final : public Allocator
	{
		struct Node
		{
			Node* next;
		};
	public:
		PoolAllocator(void* data, size_t dataSize, size_t blockSize)
		{
			m_data = data;
			m_dataSize = dataSize;
			m_blockSize = blockSize;

			InitMemory();
		}

		void* Allocate()
		{
			if (m_head == nullptr)
			{
				return nullptr;
			}

			Node* p = m_head;
			m_head = p->next;
			return p;
		}

		virtual void* Allocate(size_t size) override
		{
			ASSERT(m_blockSize == size);

			return Allocate();
		}

		void Deallocate(void* p)
		{
			Node* node = reinterpret_cast<Node*>(p);
			node->next = m_head;
			m_head = node;
		}

		virtual void Deallocate(void* p, size_t size) override
		{
			ASSERT(m_blockSize == size);

			Deallocate(p);
		}

	private:
		void* m_data;
		size_t m_dataSize;
		size_t m_blockSize;
		Node* m_head;

		void InitMemory()
		{
			ASSERT(m_dataSize >= sizeof(Node));
			m_head = nullptr;
			size_t totalBlocks = m_dataSize / m_blockSize;
			U8* p = reinterpret_cast<U8*>(m_data);
			for (size_t i = 0; i < totalBlocks; i++)
			{
				Node* n = reinterpret_cast<Node*>(p);
				n->next = m_head;
				m_head = n;
				p += m_blockSize;
			}
		}
	};
}
