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

        virtual void SetClearColor(Color c) = 0;
        virtual void SetTargetSize(int width, int height) = 0;
        virtual void AutoScale() = 0;
        virtual void SetCameraPosition(Vector2 position) = 0;
        virtual Vector2 GetCameraPosition() = 0;
        virtual Vector2 GetCameraViewSize() = 0;

        virtual void Clear() = 0;
        virtual void DrawRectangle(float x, float y, float w, float h, Color c) = 0;
        virtual void DrawImage(float x, float y, float w, float h, const Texture* img) = 0;

        virtual Texture* CreateTexture(const Bitmap& bitmap) = 0;
    };
}