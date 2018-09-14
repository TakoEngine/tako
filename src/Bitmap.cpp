#include "Bitmap.hpp"
#include "Utility.hpp"

namespace tako
{
    Bitmap::Bitmap(I32 w, I32 h) :
        width(w), height(h),
        m_data(new Color[w*h])
    {
        ASSERT(w >= 0 && h >= 0);
    }

    Color Bitmap::GetPixel(I32 x, I32 y) const
    {
        return m_data[y * width + x];
    }

    void Bitmap::SetPixel(I32 x, I32 y, Color c)
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        m_data[y * width + x] = c;
    }

    Color* Bitmap::GetData() const
    {
        return m_data.get();
    }

    void Bitmap::Clear(Color c)
    {
        for (I32 x = 0; x < width; x++)
        {
            for (I32 y = 0; y < height; y++)
            {
                m_data[y * width + x] = c;
            }
        }
    }

    void Bitmap::FillRect(I32 x, I32 y, I32 w, I32 h, Color c)
    {
        for (int i = 0; i < w; i++)
        {
            for (int j = 0; j < h; j++)
            {
                SetPixel(x + i, y + j, c);
            }
        }
    }

    void Bitmap::DrawBitmap(I32 x, I32 y, const Bitmap & bitmap)
    {
        for (int i = 0; i < bitmap.width; i++)
        {
            for (int j = 0; j < bitmap.height; j++)
            {
                SetPixel(x + i, y + j, bitmap.GetPixel(i, j));
            }
        }
    }
}