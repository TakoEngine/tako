#include "Tako.hpp"
#include "Font.hpp"

static tako::Texture* tree;
static int x = 0;
static int y = 0;
static int a = 0;
static tako::Vector2 pos(0,0);
static float delta = 0;
static tako::AudioClip* clipBump;
static tako::AudioClip* clipMiss;
static tako::Font* font;
static tako::Texture* helloText;
static int helloTextSizeX;
static int helloTextSizeY;
static std::string exampleText = "The quick brown fox jumps over the lazy dog!?";

void tako::Setup(tako::PixelArtDrawer* drawer)
{
	LOG("SANDBOX SETUP");
	//clipBump = new AudioClip("/Bump.wav");
	//clipMiss = new AudioClip("/Miss.wav");
	auto bitmap = tako::Bitmap::FromFile("/tree.png");
	tree = drawer->CreateTexture(bitmap);
	font = new tako::Font("/charmap-cellphone.png", 5, 7, 1, 1, 2, 2, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]\a_`abcdefghijklmnopqrstuvwxyz{|}~");//" !\"#$%&'()*,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]ˆ_`abcdefghijklmnopqrstuvwxyz{|}~");
    auto textBitmap = font->RenderText(exampleText, 1, 5);
    std::tie(helloTextSizeX, helloTextSizeY) = font->CalculateDimensions(exampleText, 1, 5);
	helloText = drawer->CreateTexture(textBitmap);
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
		//Audio::Play(*clipMiss);
		delta = 0;
	}
    float speed = 5;
    x = (x + 1) % 300;
    y = (y + 1) % 50;
    a = (a + 1) % 512;

    if (input->GetKey(tako::Key::W))
    {
        pos.y -= speed * dt;
    }
    if (input->GetKey(tako::Key::S))
    {
        pos.y += speed * dt;
    }
    if (input->GetKey(tako::Key::A))
    {
        pos.x -= speed * dt;
    }
    if (input->GetKey(tako::Key::D))
    {
        pos.x += speed * dt;
    }
	if (input->GetKeyDown(tako::Key::Space))
	{
		//Audio::Play(*clipBump);
	}
}

void tako::Draw(tako::PixelArtDrawer* drawer)
{
    auto alpha = static_cast<tako::U8>(PingPong(a, 255));
    drawer->Clear();

    drawer->DrawRectangle(0, 0, 100, 100, {100, 0, 255, alpha});
    drawer->DrawImage(75 + PingPong(x, 150), 75 + PingPong(y, 25), 100, 100, tree);
    drawer->DrawImage(75 + PingPong(x, 150), 175 + PingPong(y, 25), 100, 100, tree);
    drawer->DrawImage(75 + PingPong(x, 150), 275 + PingPong(y, 25), 100, 100, tree);

    drawer->DrawRectangle(pos.x, pos.y, 64, 64, {255, 0, 0, 255});
    drawer->DrawImage(-400, 0, helloTextSizeX * 2, helloTextSizeY * 2, helloText);
}
