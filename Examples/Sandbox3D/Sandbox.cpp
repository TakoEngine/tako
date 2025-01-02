#include "Tako.hpp"
#ifdef TAKO_IMGUI
#include "imgui.h"
#endif

import Tako.Renderer3D;
import Tako.FileSystem;

float PingPong(float val, float max)
{
	if (val > 2 * max)
	{
		return PingPong(val - 2 * max, max);
	}
	else if (val > max)
	{
		return 2 * max - val;
	}
	else
	{
		return val;
	}
}

struct FrameData
{
	float zoom = 1;
	tako::Vector3 trans = { 0, 2, -10 };
	tako::Vector3 lightPos = { 0, 10, -3 };
	tako::Quaternion rotation{};
	tako::Quaternion rot{ 0.99f, -0.15f, -0.02f, -0.02f };
};



class SandBoxGame
{
public:
	void Setup(const tako::SetupData& setup)
	{
		renderer = new tako::Renderer3D(setup.context);
		#ifdef TAKO_EMSCRIPTEN
		auto path = "/CrossGolf.glb";
		#else
		auto path = "./Assets/CrossGolf.glb";
		#endif
		model = renderer->LoadModel(path);
	}

	void Update(tako::Input* input, float dt, FrameData* frameData)
	{
		time += dt/10;
		if (input->GetKey(tako::Key::Down))
		{
			//rotX -= dt;
			data.rotation = data.rotation * tako::Quaternion::FromEuler({dt*4, 0, 0});
		}
		if (input->GetKey(tako::Key::Up))
		{
			//rotX += dt;
			data.rotation = data.rotation * tako::Quaternion::FromEuler({-dt*4, 0, 0});
		}
		if (input->GetKey(tako::Key::Left))
		{
			data.rotation = data.rotation * tako::Quaternion::FromEuler({0, -dt*4, 0});
		}
		if (input->GetKey(tako::Key::Right))
		{
			data.rotation = data.rotation * tako::Quaternion::FromEuler({0, dt*4, 0});
		}
		tako::Vector3 movAxis;
		if (input->GetKey(tako::Key::W))
		{
			movAxis.z += dt;
		}
		if (input->GetKey(tako::Key::S))
		{
			movAxis.z -= dt;
		}
		if (input->GetKey(tako::Key::A))
		{
			movAxis.x -= dt;
		}
		if (input->GetKey(tako::Key::D))
		{
			movAxis.x += dt;
		}
		if (input->GetKey(tako::Key::Space))
		{
			movAxis.y += dt;
		}
		if (input->GetKey(tako::Key::X))
		{
			movAxis.y -= dt;
		}


		if (input->GetKey(tako::Key::Z))
		{
			auto mouseMove = input->GetMouseMovement();
			data.rot = data.rot * tako::Quaternion::FromEuler(tako::Vector3(-mouseMove.y, -mouseMove.x, 0));
		}

		data.trans += data.rotation * movAxis;
		static float passed = 0;
		passed += dt;
		data.lightPos.x = PingPong(passed, 20) - 10;

		#ifdef TAKO_IMGUI
		ImGui::Begin("Camera");
		ImGui::InputFloat3("pos", &data.trans.x);
		ImGui::InputFloat4("rot", &data.rot.x);
		ImGui::End();
		#endif

		//Sync for renderphase
		*frameData = data;
	}

	void Draw(FrameData* frameData)
	{
		renderer->Begin();

		//renderer->SetLightPosition(frameData->lightPos);
		renderer->SetCameraView(tako::Matrix4::cameraViewMatrix(frameData->trans, frameData->rotation));
		//renderer->SetCameraView(tako::Matrix4::lookAt(frameData->trans, tako::Vector3(0,0,0), tako::Vector3(0, 1, 0)));


		auto transform = tako::Matrix4::ScaleMatrix(frameData->zoom, frameData->zoom, frameData->zoom);
		transform = transform * data.rot.ToRotationMatrix();
		renderer->DrawModel(model, transform);

		//renderer->DrawCube(tako::Matrix4::translation(0, 0, 0), model.materials[0]);

		renderer->End();
	}
private:
	tako::Renderer3D* renderer;
	tako::Model model;
	float time = 0;
	float rotX = 0;
	float rotZ = 0;
	FrameData data;
};

void Setup(void* gameData, const tako::SetupData& setup)
{
	auto game = new (gameData) SandBoxGame();
	game->Setup(setup);
}

void Update(const tako::GameStageData stageData, tako::Input* input, float dt)
{
	auto game = reinterpret_cast<SandBoxGame*>(stageData.gameData);
	game->Update(input, dt, reinterpret_cast<FrameData*>(stageData.frameData));
}

void Draw(const tako::GameStageData stageData)
{
	auto game = reinterpret_cast<SandBoxGame*>(stageData.gameData);
	game->Draw(reinterpret_cast<FrameData*>(stageData.frameData));
}

void tako::InitTakoConfig(GameConfig& config)
{
	config.Setup = Setup;
	config.Update = Update;
	config.Draw = Draw;
	config.graphicsAPI = tako::GraphicsAPI::WebGPU;
	config.initAudioDelayed = true;
	config.gameDataSize = sizeof(SandBoxGame);
	config.frameDataSize = sizeof(FrameData);
}
