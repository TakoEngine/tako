#pragma once
#include "IGraphicsContext.hpp"

namespace tako
{
	enum class BufferType
	{
		Vertex,
		Index
	};

	struct Buffer
	{
		U64 value;
	};
}
