#pragma once
#include "NumberTypes.hpp"
#include <string_view>
#include "Utility.hpp"

namespace tako
{
	namespace mathf
	{
		constexpr float PI = 3.1415927f;

		constexpr float sign(float x)
		{
			return x < 0 ? -1 : 1;
		}

		constexpr float abs(float x)
		{
			return x < 0 ? -x : x;
		}

		constexpr float clamp(float value, float min, float max)
		{
			if (value < min)
			{
				return min;
			}
			if (value > max)
			{
				return max;
			}

			return value;
		}

		constexpr float sqrt(float x)
		{
			if (x < 0) return std::numeric_limits<float>::quiet_NaN();
			float a = x / 2;
			float b = 0;
			while (a != b)
			{
				b = a;
				a = (x/a + a) / 2;
			}

			return a;
		}

		constexpr float toRad(float deg)
		{
			return deg * PI / 180;
		}
	}

	struct Vector2
	{
		float x, y;

		constexpr Vector2() : x(0), y(0) {}
		constexpr Vector2(float x, float y) : x(x), y(y) {}

		constexpr bool operator==(const Vector2& rhs) const
		{
			return x == rhs.x && y == rhs.y;
		}

		constexpr bool operator!=(const Vector2& rhs) const
		{
			return !operator==(rhs);
		}

		constexpr Vector2& operator+=(const Vector2& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		constexpr Vector2& operator-=(const Vector2& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		friend constexpr Vector2 operator+(Vector2 lhs, const Vector2 rhs)
		{
			return lhs += rhs;
		}

		friend constexpr Vector2 operator-(Vector2 lhs, const Vector2 rhs)
		{
			return lhs -= rhs;
		}

		friend constexpr Vector2 operator*(Vector2 lhs, const float rhs)
		{
			lhs.x *= rhs;
			lhs.y *= rhs;
			return lhs;
		}

		friend constexpr Vector2 operator/(Vector2 lhs, const float rhs)
		{
			lhs.x /= rhs;
			lhs.y /= rhs;
			return lhs;
		}

		constexpr Vector2& operator/=(const float factor)
		{
			x /= factor;
			y /= factor;
			return *this;
		}

		constexpr float magnitude() const
		{
			return mathf::sqrt(x * x + y * y);
		}

		Vector2& normalize()
		{
			float mag = magnitude();
			if (mag < 0.0001f)
			{
				return *this = Vector2(0, 0);
			}

			return operator/=(mag);
		}

		static Vector2 Normalized(tako::Vector2 v)
		{
			return v.normalize();
		}
	};

	struct Vector3
	{
		float x, y, z;

		constexpr Vector3() : x(0), y(0), z(0) {}
		constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

		constexpr bool operator==(const Vector3& rhs) const
		{
			return x == rhs.x && y == rhs.y && z == rhs.z;
		}

		constexpr bool operator!=(const Vector3& rhs) const
		{
			return !operator==(rhs);
		}

		constexpr Vector3& operator+=(const Vector3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		friend constexpr Vector3 operator+(Vector3 lhs, const Vector3 rhs)
		{
			return lhs += rhs;
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

		friend constexpr Vector3 operator*(const float factor, Vector3 v)
		{
			return { v.x * factor, v.y * factor, v.z * factor };
		}

		constexpr Vector3& operator*=(const float factor)
		{
			x *= factor;
			y *= factor;
			z *= factor;
			return *this;
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
			return mathf::sqrt(x * x + y * y + z * z);
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

		constexpr static float dot(const Vector3& lhs, const Vector3& rhs)
		{
			return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
		}
	};

	struct Vector4
	{
		float x, y, z, w;

		constexpr Vector4() : x(0), y(0), z(0), w(0) {}
		constexpr Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	};

	struct Quaternion;
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

		constexpr static Matrix4 ScaleMatrix(float x, float y, float z)
		{
			Matrix4 m = identity;
			m[0] *= x;
			m[5] *= y;
			m[10] *= z;
			return m;
		}

		constexpr Matrix4& scale(float x, float y, float z)
		{
			m[0] *= x;
			m[5] *= y;
			m[10] *= z;
			return *this;
		}

		constexpr static Matrix4 transpose(const Matrix4& m)
		{
			return Matrix4
			{
				m[0], m[4], m[8], m[12],
				m[1], m[5], m[9], m[13],
				m[2], m[6], m[10], m[14],
				m[3], m[7], m[11], m[15],
			};
		}

		constexpr static Matrix4 inverse(const Matrix4& m)
		{
			Matrix4 inv;

			inv[0] =
				m[5]  * m[10] * m[15] -
				m[5]  * m[11] * m[14] -
				m[9]  * m[6]  * m[15] +
				m[9]  * m[7]  * m[14] +
				m[13] * m[6]  * m[11] -
				m[13] * m[7]  * m[10];

			inv[4] =
				-m[4]  * m[10] * m[15] +
				m[4]  * m[11] * m[14] +
				m[8]  * m[6]  * m[15] -
				m[8]  * m[7]  * m[14] -
				m[12] * m[6]  * m[11] +
				m[12] * m[7]  * m[10];

			inv[8] = m[4]  * m[9] * m[15] -
				m[4]  * m[11] * m[13] -
				m[8]  * m[5] * m[15] +
				m[8]  * m[7] * m[13] +
				m[12] * m[5] * m[11] -
				m[12] * m[7] * m[9];

			inv[12] = -m[4]  * m[9] * m[14] +
				m[4]  * m[10] * m[13] +
				m[8]  * m[5] * m[14] -
				m[8]  * m[6] * m[13] -
				m[12] * m[5] * m[10] +
				m[12] * m[6] * m[9];

			inv[1] =
				-m[1]  * m[10] * m[15] +
				m[1]  * m[11] * m[14] +
				m[9]  * m[2] * m[15] -
				m[9]  * m[3] * m[14] -
				m[13] * m[2] * m[11] +
				m[13] * m[3] * m[10];

		    inv[5] =
				m[0]  * m[10] * m[15] -
				m[0]  * m[11] * m[14] -
				m[8]  * m[2] * m[15] +
				m[8]  * m[3] * m[14] +
				m[12] * m[2] * m[11] -
				m[12] * m[3] * m[10];

		    inv[9] =
				-m[0]  * m[9] * m[15] +
				m[0]  * m[11] * m[13] +
				m[8]  * m[1] * m[15] -
				m[8]  * m[3] * m[13] -
				m[12] * m[1] * m[11] +
				m[12] * m[3] * m[9];

			inv[13] =
				m[0]  * m[9] * m[14] -
				m[0]  * m[10] * m[13] -
				m[8]  * m[1] * m[14] +
				m[8]  * m[2] * m[13] +
				m[12] * m[1] * m[10] -
				m[12] * m[2] * m[9];

			inv[2] =
				m[1]  * m[6] * m[15] -
				m[1]  * m[7] * m[14] -
				m[5]  * m[2] * m[15] +
				m[5]  * m[3] * m[14] +
				m[13] * m[2] * m[7] -
				m[13] * m[3] * m[6];

			inv[6] =
				-m[0]  * m[6] * m[15] +
				m[0]  * m[7] * m[14] +
				m[4]  * m[2] * m[15] -
				m[4]  * m[3] * m[14] -
				m[12] * m[2] * m[7] +
				m[12] * m[3] * m[6];

			inv[10] =
				m[0]  * m[5] * m[15] -
				m[0]  * m[7] * m[13] -
				m[4]  * m[1] * m[15] +
				m[4]  * m[3] * m[13] +
				m[12] * m[1] * m[7] -
				m[12] * m[3] * m[5];

			inv[14] =
				-m[0]  * m[5] * m[14] +
				m[0]  * m[6] * m[13] +
				m[4]  * m[1] * m[14] -
				m[4]  * m[2] * m[13] -
				m[12] * m[1] * m[6] +
				m[12] * m[2] * m[5];

			inv[3] =
				-m[1] * m[6] * m[11] +
				m[1] * m[7] * m[10] +
				m[5] * m[2] * m[11] -
				m[5] * m[3] * m[10] -
				m[9] * m[2] * m[7] +
				m[9] * m[3] * m[6];

			inv[7] =
				m[0] * m[6] * m[11] -
				m[0] * m[7] * m[10] -
				m[4] * m[2] * m[11] +
				m[4] * m[3] * m[10] +
				m[8] * m[2] * m[7] -
				m[8] * m[3] * m[6];

			inv[11] =
				-m[0] * m[5] * m[11] +
				m[0] * m[7] * m[9] +
				m[4] * m[1] * m[11] -
				m[4] * m[3] * m[9] -
				m[8] * m[1] * m[7] +
				m[8] * m[3] * m[5];

			inv[15] =
				m[0] * m[5] * m[10] -
				m[0] * m[6] * m[9] -
				m[4] * m[1] * m[10] +
				m[4] * m[2] * m[9] +
				m[8] * m[1] * m[6] -
				m[8] * m[2] * m[5];

			float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
			if (det == 0)
			{
				return {};
			}

			det = 1.0 / det;
			Matrix4 out;
			for (int i = 0; i < 16; i++)
			{
				out[i] = inv[i] * det;
			}
			return out;
		}

		friend constexpr Matrix4 operator*(const Matrix4 lhs, const Matrix4 rhs)
		{
			Matrix4 res;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					float sum = 0;
					for (int x = 0; x < 4; x++)
					{
						sum += lhs[i * 4 + x] * rhs[j + x * 4];
					}

					res[i + j * 4] = sum;
				}
			}
			return res;
		}

		static Matrix4 rotate(float angle)
		{
			return Matrix4
			(
				1, 0, 0, 0,
				0, cos(angle), -sin(angle), 0,
				0, sin(angle), cos(angle), 0,
				0, 0, 0, 1
			);
		}

		constexpr static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& upDir)
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

		static Matrix4 cameraViewMatrix(const Vector3 position, const Quaternion& rotation);

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

		constexpr static Matrix4 ortho(float left, float right, float bottom, float top, float nearDistance, float farDistance)
		{
			Matrix4 m = Matrix4::identity;
			m[0] = 2 / (right - left);
			m[3] = -(right + left) / (right - left);
			m[5] = 2 / (top - bottom);
			m[7] = -(top + bottom) / (top - bottom);
			m[10] = -2 / (farDistance - nearDistance);
			m[11] = -(farDistance + nearDistance) / (farDistance - nearDistance);

			return m;
		}

		void Print();
	};

	struct Quaternion
	{
		float x, y, z, w;
		constexpr Quaternion() : x(0), y(0), z(0), w(1) {}
		constexpr Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

		constexpr Matrix4 ToRotationMatrix() const
		{
			return Matrix4
			(
				1-2*z*z-2*w*w, 2*y*z-2*x*w,   2*y*w+2*x*z,   0,
				2*y*z+2*x*w,   1-2*y*y-2*w*w, 2*z*w-2*x*y,   0,
				2*y*w-2*x*z,   2*z*w+2*x*y,   1-2*y*y-2*z*z, 0,
				0,0,0,1
			);
		}

		static Quaternion FromEuler(Vector3 euler)
		{
			euler *= mathf::PI / 180;
			return
			{
				std::sin(euler.z/2) * std::cos(euler.y/2) * std::cos(euler.x/2) - std::cos(euler.z/2) * std::sin(euler.y/2) * std::sin(euler.x/2),
				std::cos(euler.z/2) * std::sin(euler.y/2) * std::cos(euler.x/2) + std::sin(euler.z/2) * std::cos(euler.y/2) * std::sin(euler.x/2),
				std::cos(euler.z/2) * std::cos(euler.y/2) * std::sin(euler.x/2) - std::sin(euler.z/2) * std::sin(euler.y/2) * std::cos(euler.x/2),
				std::cos(euler.z/2) * std::cos(euler.y/2) * std::cos(euler.x/2) + std::sin(euler.z/2) * std::sin(euler.y/2) * std::sin(euler.x/2),
			};
		}

		static Quaternion AngleAxis(float degrees, Vector3 axis)
		{
			Quaternion res;
			auto radians = degrees * mathf::PI / 180;
			auto ax = std::sin(radians * 0.5f) * axis.normalize();
			res.x = ax.x;
			res.y = ax.y;
			res.z = ax.z;
			res.w = std::cos(radians * 0.5f);

			return Normalize(res);
		}

		static Quaternion Rotation(float deg, Vector3 axis)
		{
			axis.normalize();
			deg = mathf::toRad(deg / 2);
			float sd = std::sin(deg);
			return { std::cos(deg), axis.x * sd , axis.y * sd, axis.z * sd };
		}

		friend constexpr Quaternion operator*(const Quaternion& q, const Quaternion& r)
		{
			return Quaternion(
				q.x * r.w + q.w * r.x + q.y * r.z - q.z * r.y,
				q.y * r.w + q.w * r.y + q.z * r.x - q.x * r.z,
				q.z * r.w + q.w * r.z + q.x * r.y - q.y * r.x,
				q.w * r.w - q.x * r.x - q.y * r.y - q.z * r.z
			);
		}


		friend constexpr Vector3 operator*(const Quaternion& rotation, const Vector3& point)
		{
			Vector3 u(rotation.x, rotation.y, rotation.z);
			float s = rotation.w;
			return
				2.0f * Vector3::dot(u, point) * u
				+ (s*s - Vector3::dot(u, u)) * point
				+ 2.0f * s * Vector3::cross(u, point);
		}

		static Quaternion Normalize(const Quaternion& q)
		{
			float normal = std::sqrt(Dot(q, q));
			return {q.x / normal, q.y / normal, q.z / normal, q.w / normal};
		}

		static constexpr float Dot(const Quaternion& q1, const Quaternion& q2)
		{
			return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
		}

		static Quaternion RotateTowards(const Quaternion& a, const Quaternion& b, float maxDelta)
		{
			float dot = Dot(a, b);
			float angle = std::acos(std::min(std::abs(dot), 1.0f)) * 2;

			if (angle == 0)
			{
				return b;
			}
			return Slerp(a, b, maxDelta / angle);
		}

		static Quaternion Slerp(const Quaternion& from, const Quaternion& target, float t)
		{
			auto a = Normalize(from);
			auto b = Normalize(target);

			float dot = Dot(a, b);

			if (dot < 0)
			{
				b.x *= -1;
				b.y *= -1;
				b.z *= -1;
				b.w *= -1;
				dot *= -1;
			}

			if (dot > 0.999999f)
			{
				//TODO: linear interpolate
				return b;
			}

			float theta0 = std::acos(dot);
			float theta = theta0 * t;
			float sinTheta = std::sin(theta);
			float sinTheta0 = std::sin(theta0);

			float s0 = std::cos(theta) - dot * sinTheta / sinTheta0;
			float s1 = sinTheta / sinTheta0;

			return
			{
				a.x * s0 + b.x * s1,
				a.y * s0 + b.y * s1,
				a.z * s0 + b.z * s1,
				a.w * s0 + b.w * s1,
			};
		}
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
			os << "(" << (int)col.r << ", " << (int)col.g << ", " << (int)col.b << ", " << (int)col.a << ")";
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
