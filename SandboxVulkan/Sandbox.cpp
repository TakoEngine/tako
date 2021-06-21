#include "Tako.hpp"
#include "Renderer3D.hpp"

class SandBoxGame
{
public:
	void Setup(const tako::SetupData& setup)
	{
		renderer = new tako::Renderer3D(setup.context);
	}

	void Update(tako::Input* input, float dt)
	{
		time += dt/10;
	}

	void Draw()
	{
		renderer->DrawCube(tako::Matrix4::identity);
		renderer->DrawCube(tako::Matrix4::identity.translation(4, 2, 0).scale(0.5f, 0.5f, 0.5f));
		renderer->DrawCube(tako::Matrix4::identity.translation(-4, 2, 0).scale(time, time, time));
	}
private:
	tako::Renderer3D* renderer;
	float time = 0;
};

void Setup(void* gameData, const tako::SetupData& setup)
{
	auto game = new (gameData) SandBoxGame();
	game->Setup(setup);
}

void Update(void* gameData, tako::Input* input, float dt)
{
	auto game = reinterpret_cast<SandBoxGame*>(gameData);
	game->Update(input, dt);
}

void Draw(void* gameData)
{
	auto game = reinterpret_cast<SandBoxGame*>(gameData);
	game->Draw();
}

void tako::InitTakoConfig(GameConfig& config)
{
	config.Setup = Setup;
	config.Update = Update;
	config.Draw = Draw;
	config.gameDataSize = sizeof(SandBoxGame);
}
