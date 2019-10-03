#pragma once

#include "Math.hpp"
#include "Bitmap.hpp"
#include "Texture.hpp"

namespace tako
{
    class PixelArtDrawer
    {
    public:
        virtual ~PixelArtDrawer() {}

        virtual void Clear() = 0;
        virtual void DrawRectangle(float x, float y, float w, float h, Color c) = 0;
        virtual void DrawImage(float x, float y, float w, float h, const Texture* img) = 0;

        virtual Texture* CreateTexture(const Bitmap& bitmap) = 0;
    };
}