#pragma once
#include "Resources.hpp"

import Tako.Input;
import Tako.Audio;

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
		size_t frameDataSize;
	};

	struct GameConfig
	{
		void (*Setup)(void* gameData, const SetupData& setup);
		void (*CheckFrameDataSizeChange)(void* gameData, size_t& frameDataSize);
		void (*Update)(const GameStageData stageData, Input* input, float dt);
		void (*Draw)(const GameStageData stageData);
		size_t gameDataSize;
		size_t frameDataSize;
		GraphicsAPI graphicsAPI = GraphicsAPI::Default;
		bool initAudioDelayed;
	};
}
