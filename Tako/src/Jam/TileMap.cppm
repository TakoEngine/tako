#include <vector>
#include <map>
#include "OpenGLPixelArtDrawer.hpp"
#include <nlohmann/json.hpp>

export module Tako.TileMap;

namespace tako::Jam
{
	export struct TileInformation
	{

	};

	export struct TileEntity
	{
		std::string typeName;
		Vector2 position;
		Vector2 size;
		nlohmann::json fields;
	};

	export struct TileLayer
	{
		std::string name;
		Bitmap composite;
	};

	export struct TileMap
	{
		std::string name;
		Color backgroundColor;
		std::vector<TileLayer> tileLayers;
		std::vector<TileEntity> entities;
		std::vector<int> collision;
		std::vector<int> neighbours;
		int entityLayerIndex;
		int worldX;
		int worldY;
		Vector2 size;
	};

	export struct TileWorld
	{
		std::map<int, TileMap> levels;
	};
}
