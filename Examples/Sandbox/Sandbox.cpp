#include "Event.hpp"
#include "Tako.hpp"
#include "Font.hpp"
#ifdef TAKO_OPENGL
#include "OpenGLPixelArtDrawer.hpp"
#endif
#ifdef TAKO_IMGUI
#include "imgui.h"
#endif
#include "Serialization.hpp"

import Tako.Audio;

static tako::Texture tree;
static tako::Texture tileset;
static tako::Sprite* sprite;
static int x = 0;
static int y = 0;
static int a = 0;
static tako::Vector2 pos(0,0);
static float delta = 0;
static tako::AudioClip* clipMiss;
static tako::AudioClip* clipMusic;
static tako::Font* font;
static tako::Texture helloText;
static tako::Bitmap bitmap(240, 135);
static tako::Texture bufferTex;
static int helloTextSizeX;
static int helloTextSizeY;
static tako::OpenGLPixelArtDrawer* g_drawer;
static tako::GraphicsContext* g_context;
static std::string exampleText = "The quick brown fox jumps over the lazy dog!?";
static tako::Vector2 mousePos = tako::Vector2(0, 0);
static tako::Audio* g_audio;

struct GameData
{
	bool audioInited = false;
};

void InitAudio(GameData* gameData, tako::Audio* audio)
{
	LOG("Manual Audio init!");
	audio->Init();
	clipMiss = audio->Load("/Miss.wav");
	clipMusic = audio->Load("/garden-of-kittens.mp3");
	tako::Audio::Play(clipMusic, true);
	gameData->audioInited = true;
}

void Setup(void* gameDataPtr, const tako::SetupData& setup)
{
	auto* gameData = reinterpret_cast<GameData*>(gameDataPtr);
	LOG("SANDBOX SETUP");
#ifndef TAKO_EMSCRIPTEN
	InitAudio(gameData, setup.audio);
#endif
	g_audio = setup.audio;
	g_drawer = new tako::OpenGLPixelArtDrawer(setup.context);
	g_context = setup.context;
	tree = setup.resources->Load<tako::Texture>("/tree.png");
	tileset = setup.resources->Load<tako::Texture>("/Tileset.png");
	sprite = g_drawer->CreateSprite(tileset, 16, 0, 16, 16);
	font = new tako::Font("/charmap-cellphone.png", 5, 7, 1, 1, 2, 2, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]\a_`abcdefghijklmnopqrstuvwxyz{|}~");//" !\"#$%&'()*,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]ˆ_`abcdefghijklmnopqrstuvwxyz{|}~");
	auto textBitmap = font->RenderText(exampleText, 1, 5);
	std::tie(helloTextSizeX, helloTextSizeY) = font->CalculateDimensions(exampleText, 1, 5);
	helloText = g_drawer->CreateTexture(textBitmap);
	bufferTex = g_drawer->CreateTexture(bitmap);
	g_drawer->Resize(setup.context->GetWidth(), setup.context->GetHeight());
}

int PingPong(int val, int max)
{
	if (val > max)
	{
		return 2 * max - val;
	}
	else
	{
		return val;
	}
}

#ifdef TAKO_IMGUI
void ImGuiRenderComponent(void* data, const tako::Reflection::StructInformation* info)
{
	for (auto& field : info->fields)
	{
		if (auto strInfo = dynamic_cast<const tako::Reflection::StructInformation*>(field.type))
		{
			auto strData = reinterpret_cast<tako::U8*>(data) + field.offset;
			if (ImGui::TreeNode(field.name))
			{
				ImGuiRenderComponent(strData, strInfo);
				ImGui::TreePop();
			}
		}
		else
		{
			if (tako::Reflection::GetPrimitiveInformation<int>() == field.type)
			{
				auto ptr = reinterpret_cast<int*>(reinterpret_cast<tako::U8*>(data) + field.offset);
				ImGui::DragInt(field.name, ptr);
			}
			else if (tako::Reflection::GetPrimitiveInformation<bool>() == field.type)
			{
				auto ptr = reinterpret_cast<bool*>(reinterpret_cast<tako::U8*>(data) + field.offset);
				ImGui::Checkbox(field.name, ptr);
			}
		}
	}
}
#endif

struct FrameData
{

};

void Update(const tako::GameStageData stageData, tako::Input* input, float dt)
{
	auto* gameData = reinterpret_cast<GameData*>(stageData.gameData);
	if (!gameData->audioInited)
	{
		if (input->GetAnyDown())
		{
			InitAudio(gameData, g_audio);
		}
		else
		{
			return;
		}
	}

	delta += dt;
	if (delta > 1) {
		tako::Audio::Play(clipMiss);
		delta = 0;
	}
	float speed = 60;
	x = (x + 1) % 300;
	y = (y + 1) % 50;
	a = (a + 1) % 512;

	pos += input->GetAxis(tako::Axis::Left) * speed * dt;

	if (input->GetKey(tako::Key::W) || input->GetKey(tako::Key::Up) || input->GetKey(tako::Key::Gamepad_Dpad_Up))
	{
		pos.y += speed * dt;
	}
	if (input->GetKey(tako::Key::S) || input->GetKey(tako::Key::Down) || input->GetKey(tako::Key::Gamepad_Dpad_Down))
	{
		pos.y -= speed * dt;
	}
	if (input->GetKey(tako::Key::A) || input->GetKey(tako::Key::Left) || input->GetKey(tako::Key::Gamepad_Dpad_Left))
	{
		pos.x -= speed * dt;
	}
	if (input->GetKey(tako::Key::D) || input->GetKey(tako::Key::Right) || input->GetKey(tako::Key::Gamepad_Dpad_Right))
	{
		pos.x += speed * dt;
	}
	if (input->GetKeyDown(tako::Key::Space))
	{
		g_audio->Play("/Bump.wav");
	}

	static float gradOff = 0;
	static tako::Color col = {(tako::U8)(rand() % 256), (tako::U8)(rand() % 256), (tako::U8)(rand() % 256), 255};
	gradOff += dt;
	if (gradOff > 1)
	{
		gradOff--;
		col = {(tako::U8)(rand() % 256), (tako::U8)(rand() % 256), (tako::U8)(rand() % 256), 255};
	}
	for (int x = 0; x < bitmap.Width(); x++)
	{
		for (int y = 0; y < bitmap.Height(); y++)
		{
			bitmap.SetPixel(x, y, col);
		}
	}
	g_drawer->UpdateTexture(bufferTex, bitmap);
	mousePos = input->GetMousePosition();
#ifdef TAKO_IMGUI
	ImGui::Begin("Test");
	ImGui::Text("Heyo!");
	ImGui::End();
	ImGui::ShowDemoWindow();
	static tako::Serialization::TestComponent comp = {};

	ImGui::Begin("Component");
	ImGuiRenderComponent(&comp, tako::Reflection::Resolver::Get<decltype(comp)>());
	ImGui::Text(tako::Serialization::Serialize(comp).c_str());
	ImGui::End();
#endif
}

void Draw(const tako::GameStageData stageData)
{
	g_drawer->Resize(g_context->GetWidth(), g_context->GetHeight());
	auto alpha = static_cast<tako::U8>(PingPong(a, 255));
	auto drawer = g_drawer;
	//drawer->Begin();
	drawer->Clear();

	drawer->DrawImage(-200, -200, 48 * 2, 64 * 2, tileset.handle);
	drawer->DrawSprite(-300, 300, 16 * 10, 16 * 10, sprite);
	//drawer->DrawRectangle(0, 0, 100, 100, {100, 0, 255, alpha});
	drawer->DrawImage(0, 0, 240, 135, bufferTex.handle);
	drawer->DrawImage(75 + PingPong(x, 150), 75 + PingPong(y, 25), 100, 100, tree.handle);
	drawer->DrawImage(75 + PingPong(x, 150), 175 + PingPong(y, 25), 100, 100, tree.handle);
	drawer->DrawImage(75 + PingPong(x, 150), 275 + PingPong(y, 25), 100, 100, tree.handle);

	drawer->DrawRectangle(pos.x, pos.y, 64, 64, {255, 0, 0, 255});
	drawer->DrawImage(-400, 0, helloTextSizeX * 2, helloTextSizeY * 2, helloText.handle);

	auto cursorPos = mousePos + drawer->GetCameraPosition() - drawer->GetCameraViewSize() / 2;
	drawer->DrawRectangle(cursorPos.x, cursorPos.y, 16, 16, {128, 128, 128, 255});
	//drawer->End();
}

void tako::InitTakoConfig(GameConfig& config)
{
	config.Setup = Setup;
	config.Update = Update;
	config.Draw = Draw;
	config.graphicsAPI = tako::GraphicsAPI::OpenGL;
	config.initAudioDelayed = true;
	config.gameDataSize = sizeof(GameData);
	config.frameDataSize = sizeof(FrameData);
}
