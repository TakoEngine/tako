#pragma once
#include "Utility.hpp"
#include "Bitmap.hpp"
#include <map>
#include <utility>
#include <string_view>

namespace tako
{
    class Font
    {
    public:
        Font(const char* filePath, I32 width, I32 height, I32 xOff, I32 yOff, I32 xSkip, I32 ySkip, std::string_view charSet) : m_source(Bitmap::FromFile(filePath))
        {
            m_width = width;
            m_height = height;
            I32 charsPerLine = (m_source.Width()-xOff) / (m_width + xSkip);

            for (int i = 0; i < charSet.length(); i++)
            {
                I32 x = i % charsPerLine;
                I32 y = i / charsPerLine;

                m_charMap[charSet[i]] = {xOff + x * (m_width + xSkip), yOff + y * (m_height + ySkip)};
            }
        }

        Bitmap RenderText(std::string_view text, int xDistance, int charsPerLine = -1, int yDistance = 1)
        {
            auto [xSize, ySize] = CalculateDimensions(text, xDistance, charsPerLine, yDistance);
            Bitmap output(xSize, ySize);
            output.Clear({0, 0, 0, 255});
            for (int i = 0; i < text.length(); i++)
            {
                auto search = m_charMap.find(text[i]);
                if (search == m_charMap.end())
                {
                    continue;
                }
                auto [xb, yb] = search->second;
                auto xI = i;
                auto yI = 0;
                if (charsPerLine > 0)
                {
                    xI = i % charsPerLine;
                    yI = i / charsPerLine;
                }
                auto x = xI * (m_width + xDistance);
                auto y = yI * (m_height + yDistance);
                output.DrawBitmap(x, y, xb, yb, m_width, m_height, m_source);
            }
            return std::move(output);
        }

        std::pair<I32, I32> CalculateDimensions(std::string_view text, int xDistance, int charsPerLine = -1, int yDistance = 1)
        {
            auto len = text.length();
            if (charsPerLine <= 0)
            {
                return { len * (m_width + xDistance), m_height };
            }
            else
            {
                auto xLen = len > charsPerLine ? charsPerLine : len;
                auto yLen = 1 + len / charsPerLine;
                return { xLen * (m_width + xDistance), yLen * (m_height + yDistance) };
            }
        }
    private:
        Bitmap m_source;
        I32 m_width;
        I32 m_height;
        std::map<char, std::pair<I32, I32>> m_charMap;
    };
}