#include "Tako.hpp"

static tako::Texture* tree;
static int x = 0;
static int y = 0;
static int a = 0;
static tako::Vector2 pos(0,0);
static float delta = 0;
static tako::AudioClip* clipBump;
static tako::AudioClip* clipMiss;

void tako::Setup(tako::PixelArtDrawer* drawer)
{
	LOG("SANDBOX SETUP");
	//clipBump = new AudioClip("/Bump.wav");
	//clipMiss = new AudioClip("/Miss.wav");
	auto bitmap = tako::Bitmap::FromFile("./tree.png");
	tree = drawer->CreateTexture(bitmap);
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
}
