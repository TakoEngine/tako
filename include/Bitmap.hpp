#pragma once
#include "Math.hpp"
#include <memory>

namespace tako
{
    class Bitmap
    {
    public:
        const I32 width, height;

        Bitmap(I32 w, I32 h);
        Color GetPixel(I32 x, I32 y) const;
        void SetPixel(I32 x, I32 y, Color c);
        Color* GetData() const;

        void Clear(Color c);
        void FillRect(I32 x, I32 y, I32 w, I32 h, Color c);
        void DrawBitmap(I32 x, I32 y, const Bitmap& bitmap);
    private:
        std::unique_ptr<Color[]> m_data;
    };
}
