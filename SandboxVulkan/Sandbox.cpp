#include "Tako.hpp"
#include "Renderer3D.hpp"

class SandBoxGame
{
public:
	void Setup(const tako::SetupData& setup)
	{
		renderer = new tako::Renderer3D(setup.context);
		model = renderer->LoadModel("./Assets/CrossGolf.glb");
	}

	void Update(tako::Input* input, float dt)
	{
		time += dt/10;
		if (input->GetKey(tako::Key::Down))
		{
			//rotX -= dt;
			rotation = rotation * tako::Quaternion::FromEuler({dt, 0, 0});
		}
		if (input->GetKey(tako::Key::Up))
		{
			//rotX += dt;
			rotation = rotation * tako::Quaternion::FromEuler({-dt, 0, 0});
		}
		if (input->GetKey(tako::Key::Left))
		{
			rotation = rotation * tako::Quaternion::FromEuler({0, -dt, 0});
		}
		if (input->GetKey(tako::Key::Right))
		{
			rotation = rotation * tako::Quaternion::FromEuler({0, dt, 0});
		}
		tako::Vector3 movAxis;
		if (input->GetKey(tako::Key::W))
		{
			movAxis.z -= dt;
		}
		if (input->GetKey(tako::Key::S))
		{
			movAxis.z += dt;
		}
		if (input->GetKey(tako::Key::A))
		{
			movAxis.x += dt;
		}
		if (input->GetKey(tako::Key::D))
		{
			movAxis.x -= dt;
		}

		trans += rotation * movAxis;
	}

	void Draw()
	{
		renderer->SetCameraView(tako::Matrix4::cameraViewMatrix(trans, rotation));
		auto transform = tako::Matrix4::scale(zoom, zoom, zoom) * tako::Quaternion::Rotation(180, { 0,0,1 }).ToRotationMatrix();
		//renderer->DrawMesh(golf, texture, );
		renderer->DrawModel(model, transform);
	}
private:
	tako::Renderer3D* renderer;
	float time = 0;
	float rotX = 0;
	float rotZ = 0;
	float zoom = 1;
	tako::Vector3 trans = {0, 2, 0};
	tako::Quaternion rotation = tako::Quaternion::Rotation(180, { 0,1,0 });
	tako::Model model;
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
