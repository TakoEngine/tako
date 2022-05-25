#pragma once

namespace tako
{
	class Allocator
	{
	public:
		virtual ~Allocator() {};
		virtual void* Allocate(size_t size) = 0;
		virtual void Deallocate(void* p, size_t size) = 0;
	};
}
