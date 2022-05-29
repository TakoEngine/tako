#pragma once
#include <memory_resource>

namespace tako
{
	class Allocator : public std::pmr::memory_resource
	{
	public:
		virtual ~Allocator() {};
		virtual void* Allocate(size_t size) = 0;
		virtual void Deallocate(void* p, size_t size) = 0;

		void* do_allocate(size_t bytes, size_t alignment) override
		{
			return Allocate(bytes);
		}

		void do_deallocate(void* ptr, size_t bytes, size_t alignment) override
		{
			return Deallocate(ptr, bytes);
		}

		bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
		{
			return this == &other;
		}
	};
}
