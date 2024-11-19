#include "Tako.hpp"
#ifdef TAKO_OPENGL
#include "OpenGLPixelArtDrawer.hpp"
#endif
#include <vector>
#include <time.h>
#include <stdlib.h>

import Tako.PlatformerPhysics2D;

struct PhysicsObject
{
	tako::Vector2 position;
	tako::Vector2 velocity;
	tako::Jam::PlatformerPhysics2D::Rect bounds;
	bool stationary;
};

class JamPhysics2DGame
{
public:
	void Setup(const tako::SetupData& setup)
	{
		srand(time(NULL));
		m_drawer = new tako::OpenGLPixelArtDrawer(setup.context);
		m_context = setup.context;
		m_drawer->Resize(1024,768);

		/*
		for (int i = 0; i < 100; i++)
		{
			m_nodes.push_back(
			{
				tako::Vector2(rand() % 600 - 30, rand() % 200),
				tako::Vector2(rand() % 20 - 10, rand() % 100),
				{0, 0, 16, 16},
				false
			});
		}
		*/

		m_nodes.push_back(
		{
				tako::Vector2(0, -100),
				tako::Vector2(0, 0),
				{0, 0, 200, 16},
				true
		});

		m_nodes.push_back(
		{
				tako::Vector2(-250, -100),
				tako::Vector2(0, 0),
				{0, 0, 200, 16},
				true
		});

		m_nodes.push_back(
		{
				tako::Vector2(250, -100),
				tako::Vector2(0, 0),
				{0, 0, 200, 16},
				true
		});

		m_nodes.push_back(
		{
				tako::Vector2(0, -200),
				tako::Vector2(0, 0),
				{0, 0, 2000, 16},
				true
		});
	}

	void Update(tako::Input* input, float dt)
	{
		spawnDelta += dt;
		if (spawnDelta > 1)
		{
			m_nodes.push_back(
			{
					tako::Vector2(rand() % 600 - 300, rand() % 200),
					tako::Vector2(rand() % 20 - 10, rand() % 100),
					{0, 0, 16, 16},
					false
			});
			spawnDelta = 0;
		}
		for (auto& node : m_nodes)
		{
			if (!node.stationary) node.velocity.y -= dt * 10;
		}
		static std::vector<tako::Jam::PlatformerPhysics2D::Node> nodes;
		nodes.reserve(m_nodes.size());
		nodes.clear();
		for (auto& node : m_nodes)
		{
			nodes.push_back(
			{
				node.position,
				node.velocity,
				node.bounds,
				nullptr,
				{0, 0}
			});
		}
		tako::Jam::PlatformerPhysics2D::CalculateMovement(dt, nodes);
		tako::Jam::PlatformerPhysics2D::SimulatePhysics(nodes, m_tilemap, [](auto& self, auto& other) { LOG("col!");});
	}

	void Draw()
	{
		auto drawer = m_drawer;
		m_context->Begin();
		drawer->Clear();

		for (const auto& node : m_nodes)
		{
			auto color = node.stationary ? tako::Color(0, 255, 0, 255) : tako::Color(255, 0, 0, 255);
			drawer->DrawRectangle(node.position.x - node.bounds.w / 2, node.position.y - node.bounds.h / 2, node.bounds.w, node.bounds.h, color);
		}

		m_context->End();
	}
private:
	tako::OpenGLPixelArtDrawer* m_drawer;
	tako::GraphicsContext* m_context;
	std::vector<PhysicsObject> m_nodes;
	std::vector<int> m_tiles;
	tako::Jam::PlatformerPhysics2D::TileCollisionMap m_tilemap
	{
		m_tiles,
		{16, 16},
		0,
		0
	};
	float spawnDelta = 0;
};

void Setup(void* gameData, const tako::SetupData& setup)
{
	auto game = new (gameData) JamPhysics2DGame();
	game->Setup(setup);
}

void Update(const tako::GameStageData stageData, tako::Input* input, float dt)
{
	auto game = reinterpret_cast<JamPhysics2DGame*>(stageData.gameData);
	game->Update(input, dt);
}

void Draw(const tako::GameStageData stageData)
{
	auto game = reinterpret_cast<JamPhysics2DGame*>(stageData.gameData);
	game->Draw();
}

void tako::InitTakoConfig(GameConfig& config)
{
	config.Setup = Setup;
	config.Update = Update;
	config.Draw = Draw;
	config.graphicsAPI = tako::GraphicsAPI::OpenGL;
	config.gameDataSize = sizeof(JamPhysics2DGame);
	config.frameDataSize = 1;
}
