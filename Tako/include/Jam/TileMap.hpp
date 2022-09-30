#pragma once
#include <vector>
#include <map>
#include "OpenGLPixelArtDrawer.hpp"

namespace tako::Jam
{
	struct TileInformation
	{

	};

	struct TileLayer
	{
		std::string name;
		Bitmap composite;
	};

	struct TileMap
	{
		std::string name;
		Color backgroundColor;
		std::vector<TileLayer> tileLayers;
	};

	struct TileWorld
	{
		std::map<int, TileMap> levels;
	};
}
