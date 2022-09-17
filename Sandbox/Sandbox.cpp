#include "Tako.hpp"
#include "Font.hpp"
#ifdef TAKO_OPENGL
#include "OpenGLPixelArtDrawer.hpp"
#endif

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

void Setup(void* gameData, const tako::SetupData& setup)
{
	LOG("SANDBOX SETUP");
	g_audio = setup.audio;
	g_drawer = new tako::OpenGLPixelArtDrawer(setup.context);
	g_context = setup.context;
	//clipMiss = g_audio->Load("/Miss.wav");
	//clipMusic = g_audio->Load("/garden-of-kittens.mp3");
	//tako::Audio::Play(clipMusic, true);
	tree = setup.resources->Load<tako::Texture>("/tree.png");
	tileset = setup.resources->Load<tako::Texture>("/Tileset.png");
	sprite = g_drawer->CreateSprite(tileset, 16, 0, 16, 16);
	font = new tako::Font("/charmap-cellphone.png", 5, 7, 1, 1, 2, 2, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]\a_`abcdefghijklmnopqrstuvwxyz{|}~");//" !\"#$%&'()*,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]ˆ_`abcdefghijklmnopqrstuvwxyz{|}~");
	auto textBitmap = font->RenderText(exampleText, 1, 5);
	std::tie(helloTextSizeX, helloTextSizeY) = font->CalculateDimensions(exampleText, 1, 5);
	helloText = g_drawer->CreateTexture(textBitmap);
	bufferTex = g_drawer->CreateTexture(bitmap);
	g_drawer->Resize(1024,768);
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

void Update(const tako::GameStageData stageData, tako::Input* input, float dt)
{
	delta += dt;
	if (delta > 1) {
		//tako::Audio::Play(clipMiss);
		delta = 0;
	}
	float speed = 60;
	x = (x + 1) % 300;
	y = (y + 1) % 50;
	a = (a + 1) % 512;

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
		//g_audio->Play("/Bump.wav");
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
}

void Draw(const tako::GameStageData stageData)
{
	auto alpha = static_cast<tako::U8>(PingPong(a, 255));
	auto drawer = g_drawer;
	//drawer->Begin();
	g_context->Begin();
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
	g_context->End();
}

void tako::InitTakoConfig(GameConfig& config)
{
	config.Setup = Setup;
	config.Update = Update;
	config.Draw = Draw;
	config.graphicsAPI = tako::GraphicsAPI::OpenGL;
	config.gameDataSize = 1024;
	config.frameDataSize = 1024;
}
