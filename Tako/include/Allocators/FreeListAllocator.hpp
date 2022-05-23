#pragma once
#include "NumberTypes.hpp"

namespace tako::Allocators
{
	namespace
	{
		struct Node
		{
			Node* next;
			size_t size;
		};
	}

	class FreeListAllocator
	{
	public:
		FreeListAllocator(void* data, size_t size)
		{
			m_data = data;
			m_size = size;
			m_head = reinterpret_cast<Node*>(data);
			m_head->next = nullptr;
			m_head->size = size;
		}

		void* Allocate(size_t size)
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

		void Deallocate(void* data, size_t size)
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
				return;
			}

			prev->next = d;
			d->next = cur;
		}
	private:
		void* m_data;
		size_t m_size;
		Node* m_head;
	};
}
