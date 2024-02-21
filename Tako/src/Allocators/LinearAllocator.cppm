module;
#include "Utility.hpp"
#include "NumberTypes.hpp"
export module Tako.Allocators.LinearAllocator;

import Tako.Allocators.Allocator;

namespace tako::Allocators
{
	export class LinearAllocator final : public Allocator
	{
	public:
		LinearAllocator(void* data, size_t dataSize)
		{
			m_data = data;
			m_dataSize = dataSize;
			Reset();
		}

		virtual void* Allocate(size_t size) override
		{
			auto p = m_current;
			m_current += size;
			return p;
		}

		virtual void Deallocate(void* p, size_t size) override
		{
		}

		void Reset()
		{
			m_current = reinterpret_cast<std::byte*>(m_data);
		}

	private:
		void* m_data;
		size_t m_dataSize;
		size_t m_blockSize;
		std::byte* m_current;
	};
}
