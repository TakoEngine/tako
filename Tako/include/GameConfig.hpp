#pragma once
#include "Input.hpp"
#include "Resources.hpp"

namespace tako
{
	struct SetupData
	{
		GraphicsContext* context;
		Resources* resources;
	};

	struct GameConfig
	{
		void (*Setup)(void* gameData, const SetupData& setup);
		void (*Update)(void* gameData, Input* input, float dt);
		void (*Draw)(void* gameData);
		size_t gameDataSize;
	};
}
