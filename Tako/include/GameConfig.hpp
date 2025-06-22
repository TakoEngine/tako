#pragma once

// Workaround for (possibly) https://github.com/llvm/llvm-project/issues/136289
#ifdef TAKO_EMSCRIPTEN
#undef _LIBCPP_HIDE_FROM_ABI
#define _LIBCPP_HIDE_FROM_ABI
#endif

#include "GraphicsAPI.hpp"

import Tako.Input;
import Tako.Audio;
import Tako.RmlUi;
import Tako.NumberTypes;
import Tako.Resources;
import Tako.GraphicsContext;

namespace tako
{
	struct SetupData
	{
		GraphicsContext* context;
		Resources* resources;
		Audio* audio;
		RmlUi* ui;
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
