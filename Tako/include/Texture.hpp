#pragma once

import Tako.NumberTypes;

namespace tako
{
	struct TextureHandle
	{
		U64 value;
	};
	struct Texture
	{
		TextureHandle handle;
		int width;
		int height;
	};
}
