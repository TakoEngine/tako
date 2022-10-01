#pragma once
#include <vector>
#include <map>
#include "OpenGLPixelArtDrawer.hpp"
#include <nlohmann/json.hpp>

namespace tako::Jam
{
	struct TileInformation
	{

	};

	struct TileEntity
	{
		std::string typeName;
		Vector2 position;
		Vector2 size;
		nlohmann::json fields;
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
		std::vector<TileEntity> entities;
		std::vector<int> collision;
		int entityLayerIndex;
		Vector2 size;
	};

	struct TileWorld
	{
		std::map<int, TileMap> levels;
	};
}
