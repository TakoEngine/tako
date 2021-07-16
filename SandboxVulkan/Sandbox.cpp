#include "Tako.hpp"
#include "Renderer3D.hpp"

class SandBoxGame
{
public:
	void Setup(const tako::SetupData& setup)
	{
		renderer = new tako::Renderer3D(setup.context);
		golf = renderer->LoadMesh("./Assets/CrossGolf.obj");
		texture = renderer->CreateTexture(tako::Bitmap::FromFile("/CrossGolf.png"));
	}

	void Update(tako::Input* input, float dt)
	{
		time += dt/10;
		if (input->GetKey(tako::Key::Down))
		{
			rotX += dt*10;
		}
		if (input->GetKey(tako::Key::Up))
		{
			rotX -= dt*10;
		}
		if (input->GetKey(tako::Key::Left))
		{
			rotZ += dt * 10;
		}
		if (input->GetKey(tako::Key::Right))
		{
			rotZ -= dt * 10;
		}
		if (input->GetKey(tako::Key::Right))
		{
			//zoom += dt;
		}
		if (input->GetKey(tako::Key::Left))
		{
			//zoom -= dt;
		}
		if (input->GetKey(tako::Key::W))
		{
			trans.z -= dt;
		}
		if (input->GetKey(tako::Key::S))
		{
			trans.z += dt;
		}
		if (input->GetKey(tako::Key::A))
		{
			trans.x -= dt;
		}
		if (input->GetKey(tako::Key::D))
		{
			trans.x += dt;
		}
	}

	void Draw()
	{
		auto roti = tako::Quaternion::Rotation(180, { 0,0,1 }).ToRotationMatrix() * tako::Quaternion::Rotation(rotX, { 1,0,0 }).ToRotationMatrix() * tako::Quaternion::Rotation(rotZ, { 0,1,0 }).ToRotationMatrix();
		renderer->DrawMesh(golf, texture, tako::Matrix4::translation(trans.x, trans.y, trans.z) * tako::Matrix4::scale(zoom, zoom, zoom) * roti);
	}
private:
	tako::Renderer3D* renderer;
	tako::Mesh golf;
	tako::Texture texture;
	float time = 0;
	float rotX = 0;
	float rotZ = 0;
	float zoom = 1;
	tako::Vector3 trans;
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
