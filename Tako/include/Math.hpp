#pragma once
#include "NumberTypes.hpp"
#include <string_view>
#include "Utility.hpp"

namespace tako
{
    struct Color
    {
        U8 r = 0, g = 0, b = 0, a = 0;
        constexpr Color() {}
        constexpr Color(U8 r, U8 g, U8 b, U8 a) : r(r), g(g), b(b), a(a) {}
        constexpr Color(std::string_view hexCode)
        {
            ASSERT(hexCode.size() == 7 || hexCode.size() == 9 || hexCode.size() == 4);
            if (hexCode.size() == 7 || hexCode.size() == 9)
            {
                r = ParseHex(hexCode.substr(1, 2));
                g = ParseHex(hexCode.substr(3, 2));
                b = ParseHex(hexCode.substr(5, 2));
                a = hexCode.size() == 9 ? ParseHex(hexCode.substr(7, 2)) : 255;
            }
            else if (hexCode.size() == 4)
            {
                r = ParseDoubleHex(hexCode[1]);
                g = ParseDoubleHex(hexCode[2]);
                b = ParseDoubleHex(hexCode[3]);
                a = 255;
            }
        }

        constexpr static Color FromHSL(float h, float s, float l)
        {
            if (s == 0)
            {
                U8 c = l * 255;
                return Color(c, c, c, 255);
            }

            float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            float p = 2 * l - q;

            float r = hue2rgb(p, q, h + 1.0f / 3);
            float g = hue2rgb(p, q, h);
            float b = hue2rgb(p, q, h - 1.0f / 3);

            return Color(r * 255, g * 255, b * 255, 255);
        }

        friend std::ostream& operator<<(std::ostream& os, const Color& col)
        {
            os << "(" << (int) col.r << ", " << (int) col.g << ", " << (int) col.b << ", " << (int)col.a << ")";
            return os;
        }

    private:
        constexpr static U8 ParseHex(std::string_view str)
        {
            U8 result = 0;
            for (auto c : str)
            {
                result = result * 16 + CharToNumber(c);
            }

            return result;
        }

        constexpr static U8 ParseDoubleHex(char c)
        {
            U8 num = CharToNumber(c);
            return num * 16 + num;
        }

        constexpr static U8 CharToNumber(char cha)
        {
            switch (cha)
            {
                case '0': return  0;
                case '1': return  1;
                case '2': return  2;
                case '3': return  3;
                case '4': return  4;
                case '5': return  5;
                case '6': return  6;
                case '7': return  7;
                case '8': return  8;
                case '9': return  9;
                case 'A': return 10;
                case 'B': return 11;
                case 'C': return 12;
                case 'D': return 13;
                case 'E': return 14;
                case 'F': return 15;
                default: ASSERT(false);
            }
        }

        constexpr static float hue2rgb(float p, float q, float t)
        {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1.0f / 6) return p + (q - p) * 6 * t;
            if (t < 1.0f / 2) return q;
            if (t < 2.0f / 3) return p + (q - p) * (2.0f / 3 - t) * 6;
            return p;
        }
    };

    namespace literals
    {
        constexpr Color operator "" _col(const char* hexCode, size_t size)
        {
            return Color({ hexCode, size });
        }
    }
    
}