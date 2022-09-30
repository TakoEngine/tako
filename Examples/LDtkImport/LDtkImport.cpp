#include "Jam/TileMap.hpp"
#include "Tako.hpp"
#ifdef TAKO_OPENGL
#include "OpenGLPixelArtDrawer.hpp"
#endif
#include "Jam/LDtkImporter.hpp"

struct GameData
{
	bool audioInited = false;
	tako::OpenGLPixelArtDrawer* drawer;
	tako::GraphicsContext* context;
	tako::Jam::TileWorld tileWorld;
	std::vector<tako::Texture> activeLayers;
	tako::Jam::TileMap* activeMap;
	int activeLayerCount;
	int currentLevel;
};

void SetupLevel(GameData* gameData, int id)
{
	gameData->currentLevel = id;
	auto& level = gameData->tileWorld.levels[id];
	gameData->activeMap = &level;
	for (int i = 0; i < level.tileLayers.size(); i++)
	{
		auto& layer = level.tileLayers[i];
		if (i >= gameData->activeLayers.size())
		{
			auto tex = gameData->drawer->CreateTexture(layer.composite);
			gameData->activeLayers.push_back(tex);
		}
		else
		{
			gameData->drawer->UpdateTexture(gameData->activeLayers[i], layer.composite);
		}
	}
	gameData->activeLayerCount = level.tileLayers.size();
	gameData->drawer->SetTargetSize(256, 256);
	gameData->drawer->SetCameraPosition({128,128});
}

void Setup(void* gameDataPtr, const tako::SetupData& setup)
{
	auto* gameData = reinterpret_cast<GameData*>(gameDataPtr);
	new (gameData) GameData();
	gameData->drawer = new tako::OpenGLPixelArtDrawer(setup.context);
	gameData->context = setup.context;
	gameData->tileWorld = tako::Jam::LDtkImporter::LoadWorld("/World.ldtk");
	SetupLevel(gameData, 0);
	gameData->drawer->AutoScale();
}

struct FrameData
{

};

void Update(const tako::GameStageData stageData, tako::Input* input, float dt)
{
	auto* gameData = reinterpret_cast<GameData*>(stageData.gameData);
}


void DrawTileLayer(GameData* gameData, int i)
{
	auto& tex = gameData->activeLayers[i];
	gameData->drawer->DrawImage(0, tex.height, tex.width, tex.height, gameData->activeLayers[i].handle);
}

void DrawEntities(GameData* gameData)
{
	for (auto& ent : gameData->activeMap->entities)
	{
		gameData->drawer->DrawRectangle(ent.position.x - ent.size.x / 2, ent.position.y + ent.size.y / 2, ent.size.x, ent.size.y, {255, 0, 0, 255});
	}
}

void Draw(const tako::GameStageData stageData)
{
	auto* gameData = reinterpret_cast<GameData*>(stageData.gameData);
	gameData->drawer->Resize(gameData->context->GetWidth(), gameData->context->GetHeight());
	gameData->drawer->SetClearColor(gameData->activeMap->backgroundColor);
	gameData->drawer->Clear();
	if (gameData->activeMap->entityLayerIndex < 0)
	{
		DrawEntities(gameData);
	}
	for (int i = 0; i < gameData->activeLayerCount; i++)
	{
		DrawTileLayer(gameData, i);
		if (gameData->activeMap->entityLayerIndex == i)
		{
			DrawEntities(gameData);
		}
	}
}

void tako::InitTakoConfig(GameConfig& config)
{
	config.Setup = Setup;
	config.Update = Update;
	config.Draw = Draw;
	config.graphicsAPI = tako::GraphicsAPI::OpenGL;
	config.initAudioDelayed = true;
	config.gameDataSize = sizeof(GameData);
	config.frameDataSize = sizeof(FrameData);
}
