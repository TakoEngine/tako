#pragma once
#include "Assets.hpp"
#include "Jam/TileMap.hpp"
#include <nlohmann/json.hpp>

namespace tako::Jam::LDtkImporter
{
	class Map
	{
	public:
	private:

	};
	static TileWorld LoadWorld(const char* projectFile)
	{
		auto json = nlohmann::json::parse(Assets::ReadAssetText(projectFile));

		auto& tileSets = json["defs"]["tilesets"];

		std::map<int, Bitmap> tileSetMap;

		for (auto& tileSet : tileSets)
		{
			tileSetMap[tileSet["uid"]] =  Bitmap::FromFile(("/" + tileSet["relPath"].get<std::string>()).c_str());

		}

		TileWorld world;

		for (auto& level : json["levels"])
		{
			TileMap tl;
			tl.name = level["identifier"];
			int levelWidth = level["pxWid"];
			int levelHeight = level["pxHei"];
			for (auto& layer : level["layerInstances"])
			{

				std::string type = layer["__type"];
				if (type == "Tiles")
				{
					auto& tileSet = tileSetMap[layer["__tilesetDefUid"]];
					int gridSize = layer["__gridSize"];
					TileLayer l;
					l.composite = Bitmap(levelWidth, levelHeight);
					l.composite.FillRect(0, 0, levelWidth, levelHeight, {0, 0, 0, 0});
					for (auto& tile : layer["gridTiles"])
					{
						l.composite.DrawBitmap(tile["px"][0], tile["px"][1], tile["src"][0], tile["src"][1], gridSize, gridSize, tileSet);
					}
					tl.tileLayers.emplace_back(std::move(l));
				}
			}
			tl.backgroundColor = Color(level["__bgColor"].get<std::string>());
			world.levels[level["uid"]] = std::move(tl);
		}

		return world;
	}
}
