#pragma once

#include <iostream>
//
#include "Window.hpp"
#include "GraphicsContext.hpp"
#include "Utility.hpp"
#include "FileSystem.hpp"
#include "Input.hpp"
#include "Resources.hpp"
//#include <emscripten.h>
#include "Audio.hpp"
#include "PixelArtDrawer.hpp"
#include "Renderer3D.hpp"
#include "GameConfig.hpp"
#include "JobSystem.hpp"

namespace tako
{
	//extern void Setup(PixelArtDrawer* drawer, Resources* resources);
	//extern void Update(Input* input, float dt);
	//extern void Draw(PixelArtDrawer* drawer);

	extern void InitTakoConfig(GameConfig& config);
}
