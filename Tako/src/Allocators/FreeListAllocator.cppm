module;
#include "NumberTypes.hpp"
export module Tako.Allocators.FreeListAllocator;

import Tako.Allocators.Allocator;

namespace tako::Allocators
{
	export class FreeListAllocator final : public Allocator
	{
		struct Node
		{
			Node* next;
			size_t size;
		};
	public:
		FreeListAllocator(void* data, size_t size)
		{
			m_data = data;
			m_size = size;
			m_head = reinterpret_cast<Node*>(data);
			m_head->next = nullptr;
			m_head->size = size;
		}

		virtual void* Allocate(size_t size) override
		{
			Node* cur = m_head;
			Node* prev = nullptr;
			while (cur != nullptr && cur->size != size && (cur->size + sizeof(Node)) < size)
			{
				prev = cur;
				cur = cur->next;
			}
			if (cur == nullptr)
			{
				return nullptr;
			}

			if (cur->size == size)
			{
				if (prev == nullptr)
				{
					m_head = cur->next;
				}
				else
				{
					prev->next = cur->next;
				}
				return cur;
			}

			Node* split = reinterpret_cast<Node*>(reinterpret_cast<U8*>(cur) + size);
			split->next = cur->next;
			if (prev != nullptr)
			{
				prev->next = split;
			}
			else
			{
				m_head = split;
			}
			
			split->size = cur->size - size;

			return cur;
		}

		virtual void Deallocate(void* data, size_t size) override
		{
			Node* d = reinterpret_cast<Node*>(data);
			d->size = size;

			Node* cur = m_head;
			Node* prev = nullptr;
			while (cur && cur < d)
			{
				prev = cur;
				cur = cur->next;
			}
			if (prev == nullptr)
			{
				m_head = d;
				d->next = cur;
				Merge(nullptr, m_head);
				return;
			}

			prev->next = d;
			d->next = cur;
			Merge(prev, d);
		}
	private:
		void* m_data;
		size_t m_size;
		Node* m_head;

		void Merge(Node* prev, Node* mid)
		{
			if (mid->next != nullptr && (reinterpret_cast<U8*>(mid->next) + mid->size) == reinterpret_cast<U8*>(mid->next))
			{
				mid->size += mid->next->size;
				mid->next = mid->next->next;
			}

			if (prev != nullptr && (reinterpret_cast<U8*>(prev) + prev->size) == reinterpret_cast<U8*>(mid))
			{
				prev->size += mid->size;
				prev->next = mid->next;
			}
		}
	};
}
