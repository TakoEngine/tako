#pragma once
#include "Texture.hpp"
#include <unordered_map>
#include <string>
#include <string_view>

import Tako.GraphicsContext;
import Tako.Bitmap;

namespace tako
{
	class Resources
	{
	public:
		Resources(GraphicsContext* drawer)
		{
			m_drawer = drawer;
		}

		template<typename T>
		T Load(std::string path);
	private:
		std::unordered_map<std::string, Texture> m_map;
		GraphicsContext* m_drawer;
	};
}
