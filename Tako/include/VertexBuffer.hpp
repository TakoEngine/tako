#pragma once
#include "NumberTypes.hpp"

namespace tako
{
	enum class BufferType
	{
		Vertex,
		Index,
		Uniform,
		Storage
	};

	struct Buffer
	{
		U64 value = 0;
	};
}
