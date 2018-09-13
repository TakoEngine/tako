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

    Color Bitmap::GetPixel(I32 x, I32 y)
    {
        return m_data[y * width + x];
    }

    void Bitmap::SetPixel(I32 x, I32 y, Color c)
    {
        m_data[y * width + x] = c;
    }

    Color* Bitmap::GetData()
    {
        return m_data.get();
    }
}