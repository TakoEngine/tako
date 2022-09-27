#pragma once
#include "Input.hpp"
#include "Resources.hpp"

namespace tako
{
	struct SetupData
	{
		GraphicsContext* context;
		Resources* resources;
		Audio* audio;
	};

	struct GameStageData
	{
		void* gameData;
		void* frameData;
	};

	struct GameConfig
	{
		void (*Setup)(void* gameData, const SetupData& setup);
		void (*Update)(const GameStageData stageData, Input* input, float dt);
		void (*Draw)(const GameStageData stageData);
		size_t gameDataSize;
		size_t frameDataSize;
		GraphicsAPI graphicsAPI = GraphicsAPI::Default;
		bool initAudioDelayed;
	};
}
