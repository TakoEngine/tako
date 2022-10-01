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
			auto path = tileSet["relPath"];
			if (!path.is_string()) continue;
			tileSetMap[tileSet["uid"]] =  Bitmap::FromFile(("/" + path.get<std::string>()).c_str());
		}

		TileWorld world;

		for (auto& level : json["levels"])
		{
			TileMap tl;
			tl.name = level["identifier"];
			int levelWidth = level["pxWid"];
			int levelHeight = level["pxHei"];

			int i = 0;
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
					i++;
				}
				else if (type == "Entities")
				{
					tl.entityLayerIndex = i;
					for (auto& entity : layer["entityInstances"])
					{
						TileEntity ent;
						ent.typeName = entity["__identifier"];
						ent.position = { entity["px"][0], levelHeight - entity["px"][1].get<float>()};
						ent.size = { entity["width"], entity["height"]};
						ent.fields = entity["fieldInstances"];
						tl.entities.push_back(ent);
					}
				}
				else if (type == "IntGrid")
				{
					tl.collision = layer["intGridCsv"].get<std::vector<int>>();
				}
			}

			tl.backgroundColor = Color(level["__bgColor"].get<std::string>());
			tl.entityLayerIndex = tl.tileLayers.size() - 1 - tl.entityLayerIndex;
			tl.size = { (float) levelWidth, (float) levelHeight };
			std::reverse(tl.tileLayers.begin(), tl.tileLayers.end());
			world.levels[level["uid"]] = std::move(tl);
		}
		return world;
	}
}
