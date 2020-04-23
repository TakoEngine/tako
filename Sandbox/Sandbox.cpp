#include "Tako.hpp"
#include "Font.hpp"

static tako::Texture* tree;
static tako::Texture* tileset;
static tako::Sprite* sprite;
static int x = 0;
static int y = 0;
static int a = 0;
static tako::Vector2 pos(0,0);
static float delta = 0;
static tako::AudioClip* clipBump;
static tako::AudioClip* clipMiss;
static tako::AudioClip* clipMusic;
static tako::Font* font;
static tako::Texture* helloText;
static tako::Bitmap bitmap(240, 135);
static tako::Texture* bufferTex;
static int helloTextSizeX;
static int helloTextSizeY;
static tako::PixelArtDrawer* g_drawer;
static std::string exampleText = "The quick brown fox jumps over the lazy dog!?";

void tako::Setup(tako::PixelArtDrawer* drawer)
{
	LOG("SANDBOX SETUP");
	g_drawer = drawer;
	clipBump = new AudioClip("/Bump.wav");
	clipMiss = new AudioClip("/Miss.wav");
	clipMusic = new AudioClip("/garden-of-kittens.mp3");
	tako::Audio::Play(*clipMusic, true);
    {
        auto bitmap = tako::Bitmap::FromFile("/tree.png");
        tree = drawer->CreateTexture(bitmap);
    }
    {
        auto bitmap = tako::Bitmap::FromFile("/Tileset.png");
        tileset = drawer->CreateTexture(bitmap);
    }
    sprite = drawer->CreateSprite(tileset, 16, 0, 16, 16);
	font = new tako::Font("/charmap-cellphone.png", 5, 7, 1, 1, 2, 2, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]\a_`abcdefghijklmnopqrstuvwxyz{|}~");//" !\"#$%&'()*,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]ˆ_`abcdefghijklmnopqrstuvwxyz{|}~");
    auto textBitmap = font->RenderText(exampleText, 1, 5);
    std::tie(helloTextSizeX, helloTextSizeY) = font->CalculateDimensions(exampleText, 1, 5);
	helloText = drawer->CreateTexture(textBitmap);
	bufferTex = drawer->CreateTexture(bitmap);
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

void tako::Update(tako::Input* input, float dt)
{
	delta += dt;
	if (delta > 1) {
		Audio::Play(*clipMiss);
		delta = 0;
	}
    float speed = 60;
    x = (x + 1) % 300;
    y = (y + 1) % 50;
    a = (a + 1) % 512;

    if (input->GetKey(tako::Key::W) || input->GetKey(tako::Key::Gamepad_Dpad_Up))
    {
        pos.y += speed * dt;
    }
    if (input->GetKey(tako::Key::S) || input->GetKey(tako::Key::Gamepad_Dpad_Down))
    {
        pos.y -= speed * dt;
    }
    if (input->GetKey(tako::Key::A) || input->GetKey(tako::Key::Gamepad_Dpad_Left))
    {
        pos.x -= speed * dt;
    }
    if (input->GetKey(tako::Key::D) || input->GetKey(tako::Key::Gamepad_Dpad_Right))
    {
        pos.x += speed * dt;
    }
	if (input->GetKeyDown(tako::Key::Space))
	{
		Audio::Play(*clipBump);
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
}

void tako::Draw(tako::PixelArtDrawer* drawer)
{
    auto alpha = static_cast<tako::U8>(PingPong(a, 255));
    drawer->Clear();

    drawer->DrawImage(-200, -200, 48 * 2, 64 * 2, tileset);
    drawer->DrawSprite(-300, 300, 16 * 10, 16 * 10, sprite);
    //drawer->DrawRectangle(0, 0, 100, 100, {100, 0, 255, alpha});
    drawer->DrawImage(0, 0, 240, 135, bufferTex);
    drawer->DrawImage(75 + PingPong(x, 150), 75 + PingPong(y, 25), 100, 100, tree);
    drawer->DrawImage(75 + PingPong(x, 150), 175 + PingPong(y, 25), 100, 100, tree);
    drawer->DrawImage(75 + PingPong(x, 150), 275 + PingPong(y, 25), 100, 100, tree);

    drawer->DrawRectangle(pos.x, pos.y, 64, 64, {255, 0, 0, 255});
    drawer->DrawImage(-400, 0, helloTextSizeX * 2, helloTextSizeY * 2, helloText);
}
