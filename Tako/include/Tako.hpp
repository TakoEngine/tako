#pragma once

#include <iostream>
//
#include "Window.hpp"
#include "GraphicsContext.hpp"
#include "Utility.hpp"
#include "Input.hpp"
#include "Resources.hpp"
//#include <emscripten.h>
#include "PixelArtDrawer.hpp"
#include "GameConfig.hpp"
#include "JobSystem.hpp"

namespace tako
{
	//extern void Setup(PixelArtDrawer* drawer, Resources* resources);
	//extern void Update(Input* input, float dt);
	//extern void Draw(PixelArtDrawer* drawer);

	extern void InitTakoConfig(GameConfig& config);
}
