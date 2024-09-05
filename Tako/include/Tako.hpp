#pragma once

#include <iostream>
//
#include "Window.hpp"
#include "Utility.hpp"
#include "Resources.hpp"
//#include <emscripten.h>
#include "PixelArtDrawer.hpp"
#include "GameConfig.hpp"

import Tako.Input;
import Tako.JobSystem;
import Tako.GraphicsContext;

namespace tako
{
	//extern void Setup(PixelArtDrawer* drawer, Resources* resources);
	//extern void Update(Input* input, float dt);
	//extern void Draw(PixelArtDrawer* drawer);

	extern void InitTakoConfig(GameConfig& config);
}
