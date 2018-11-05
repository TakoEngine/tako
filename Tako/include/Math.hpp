#pragma once
#include "NumberTypes.hpp"
#include <string_view>
#include "Utility.hpp"

namespace tako
{
    namespace mathf
    {
        constexpr float abs(float x)
        {
            return x < 0 ? -x : x;
        }

        //TODO: optimize
        constexpr float sqrt(float x, float margin = 0.000001f)
        {
            if (x < 0) return -1;
            float a = 1;
            float b = x;
            while (abs(a - b) < margin)
            {
                a = (a + b) / 2;
                b = x / a;
            }

            return a;
        }

        const float PI = 3.1415927f;
    }

    struct Vector2
    {
        float x, y;

        constexpr Vector2(float x, float y) : x(x), y(y) {}

        constexpr bool operator==(const Vector2& rhs) const
        {
            return x == rhs.x && y == rhs.y;
        }

        constexpr bool operator!=(const Vector2& rhs) const
        {
            return !operator==(rhs);
        }

        constexpr Vector2& operator-=(const Vector2& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        friend constexpr Vector2 operator-(Vector2 lhs, const Vector2 rhs)
        {
            return lhs -= rhs;
        }

        constexpr Vector2& operator/=(const float factor)
        {
            x /= factor;
            y /= factor;
            return *this;
        }

        constexpr float magnitude() const
        {
            return mathf::sqrt(x*x + y * y);
        }

        constexpr Vector2& normalize()
        {
            float mag = magnitude();
            if (mag < 0.0001f)
            {
                return *this = Vector2(0, 0);
            }

            return operator/=(mag);
        }
    };

    struct Vector3
    {
        float x, y, z;

        constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

        constexpr bool operator==(const Vector3& rhs) const
        {
            return x == rhs.x && y == rhs.y && z == rhs.z;
        }

        constexpr bool operator!=(const Vector3& rhs) const
        {
            return !operator==(rhs);
        }

        constexpr Vector3& operator-=(const Vector3& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        friend constexpr Vector3 operator-(Vector3 lhs, const Vector3 rhs)
        {
            return lhs -= rhs;
        }

        constexpr Vector3& operator/=(const float factor)
        {
            x /= factor;
            y /= factor;
            z /= factor;
            return *this;
        }

        constexpr float magnitude() const
        {
            return mathf::sqrt(x*x + y * y + z * z);
        }

        constexpr Vector3& normalize()
        {
            float mag = magnitude();
            if (mag < 0.0001f)
            {
                return *this = Vector3(0, 0, 0);
            }

            return operator/=(mag);
        }

        constexpr static Vector3 cross(const Vector3& lhs, const Vector3& rhs)
        {
            float x = lhs.y * rhs.z - lhs.z * rhs.y;
            float y = lhs.z * rhs.x - lhs.x * rhs.z;
            float z = lhs.x * rhs.y - lhs.y * rhs.x;

            return Vector3(x, y, z);
        }

        constexpr Vector3 cross(const Vector3& rhs) const
        {
            return Vector3::cross(*this, rhs);
        }
    };

    struct Matrix4
    {
        float m[16];

        constexpr Matrix4() : m() {}
        constexpr Matrix4(float m1, float m2, float m3, float m4,
            float m5, float m6, float m7, float m8,
            float m9, float m10, float m11, float m12,
            float m13, float m14, float m15, float m16) : m{ m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16 } {}
        static const Matrix4 identity;

        constexpr float& operator[](int i)
        {
            return m[i];
        }

        constexpr const float& operator[](int i) const
        {
            return m[i];
        }

        constexpr Matrix4& translate(float x, float y, float z)
        {
            m[12] += x;
            m[13] += y;
            m[14] += z;
            return *this;
        }

        constexpr static Matrix4 translation(float x, float y, float z)
        {
            Matrix4 mat = identity;
            mat.translate(x, y, z);
            return mat;
        }

        constexpr Matrix4& scale(float x, float y, float z)
        {
            m[0] *= x;
            m[5] *= y;
            m[10] *= z;
            return *this;
        }

        constexpr static Matrix4 lookAt(Vector3& eye, Vector3& target, Vector3& upDir)
        {
            Vector3 forward = eye - target;
            forward.normalize();

            Vector3 left = upDir.cross(forward);
            left.normalize();

            Vector3 up = forward.cross(left);

            Matrix4 matrix = Matrix4::identity;

            matrix[0] = left.x;
            matrix[4] = left.y;
            matrix[8] = left.z;
            matrix[1] = up.x;
            matrix[5] = up.y;
            matrix[9] = up.z;
            matrix[2] = forward.x;
            matrix[6] = forward.y;
            matrix[10] = forward.z;

            matrix[12] = -left.x * eye.x - left.y * eye.y - left.z * eye.z;
            matrix[13] = -up.x * eye.x - up.y * eye.y - up.z * eye.z;
            matrix[14] = -forward.x * eye.x - forward.y * eye.y - forward.z * eye.z;

            return matrix;
        }

        constexpr static Matrix4 perspective(float fov, float aspect, float nearDist, float farDist)
        {
            if (fov <= 0 || aspect == 0)
            {
                return identity;
            }

            Matrix4 result;
            float tanHalfFov = tan(fov / 2);


            result[0] = 1 / (aspect * tanHalfFov);
            result[5] = 1 / tanHalfFov;
            result[10] = farDist / (nearDist - farDist);
            result[11] = -1;
            result[14] = -(farDist * nearDist) / (farDist - nearDist);

            return result;
        }
    };

    const Matrix4 Matrix4::identity =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

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