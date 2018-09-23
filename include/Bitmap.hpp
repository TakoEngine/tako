#pragma once
#include "Math.hpp"
#include <memory>

namespace tako
{
    class Bitmap
    {
    public:
        Bitmap(I32 w, I32 h);
        Bitmap(Bitmap&& other);
        Bitmap& operator=(Bitmap&& other);

        I32 Width() const;
        I32 Height() const;
        Color GetPixel(I32 x, I32 y) const;
        void SetPixel(I32 x, I32 y, Color c);
        const Color* GetData() const;
        Color* GetData();

        void Clear(Color c);
        void FillRect(I32 x, I32 y, I32 w, I32 h, Color c);
        void DrawBitmap(I32 x, I32 y, const Bitmap& bitmap);

        Bitmap Clone() const;
    private:
        I32 m_width, m_height;
        std::unique_ptr<Color[]> m_data;
    };
}
