#include "Tako.hpp"

static tako::Texture* tree;
static int x = 0;
static int y = 0;
static int a = 0;

void tako::Setup(PixelArtDrawer* drawer)
{
	LOG("SANDBOX SETUP");
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

void tako::Update()
{
    x = (x + 1) % 300;
    y = (y + 1) % 50;
    a = (a + 1) % 512;
}

void tako::Draw(PixelArtDrawer* drawer)
{
    auto alpha = static_cast<tako::U8>(PingPong(a, 255));
    drawer->Clear();
    drawer->DrawRectangle(0, 0, 100, 100, {100, 0, 255, alpha});
    drawer->DrawImage(75 + PingPong(x, 150), 75 + PingPong(y, 25), 100, 100, tree);
    drawer->DrawImage(75 + PingPong(x, 150), 175 + PingPong(y, 25), 100, 100, tree);
    drawer->DrawImage(75 + PingPong(x, 150), 275 + PingPong(y, 25), 100, 100, tree);
}
