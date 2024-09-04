module;
#include "Utility.hpp"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include "stb_image.h"
#include <memory>
#include <cstring>
export module Tako.Bitmap;

import Tako.Math;
import Tako.Assets;
import Tako.NumberTypes;

namespace tako
{
	export class Bitmap
	{
	public:
		Bitmap(I32 w, I32 h);
		Bitmap();
		Bitmap(Bitmap&& other);
		Bitmap(const Color* data, I32 w, I32 h);
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
		void DrawBitmap(I32 x, I32 y, I32 xb, I32 yb, I32 w, I32 h, const Bitmap& bitmap);

		Bitmap Clone() const;
		static Bitmap FromFile(const char* filePath);
		static Bitmap FromFileData(const U8* data, size_t size);
	private:
		I32 m_width, m_height;
		std::unique_ptr<Color[]> m_data;
	};

	Bitmap::Bitmap(I32 w, I32 h) :
		m_width(w), m_height(h),
		m_data(new Color[w * h])
	{
		ASSERT(w >= 0 && h >= 0);
	}

	Bitmap::Bitmap() :
		m_width(1), m_height(1),
		m_data(new Color[1])
	{
	}

	Bitmap::Bitmap(Bitmap&& other) :
		m_width(other.m_width),
		m_height(other.m_height),
		m_data(std::move(other.m_data))
	{
	}

	Bitmap::Bitmap(const Color* data, I32 w, I32 h) :
		m_width(w), m_height(h),
		m_data(new Color[w * h])
	{
		std::memcpy(m_data.get(), data, m_width * m_height * sizeof(Color));
	}

	Bitmap& Bitmap::operator=(Bitmap&& other)
	{
		m_width = other.m_width;
		m_height = other.m_height;
		m_data = std::move(other.m_data);
		return *this;
	}

	I32 Bitmap::Width() const
	{
		return m_width;
	}

	I32 Bitmap::Height() const
	{
		return m_height;
	}

	Color Bitmap::GetPixel(I32 x, I32 y) const
	{
		return m_data[y * m_width + x];
	}

	void Bitmap::SetPixel(I32 x, I32 y, Color c)
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;
		m_data[y * m_width + x] = c;
	}

	const Color* Bitmap::GetData() const
	{
		return m_data.get();
	}

	Color* Bitmap::GetData()
	{
		return m_data.get();
	}

	void Bitmap::Clear(Color c)
	{
		for (I32 x = 0; x < m_width; x++)
		{
			for (I32 y = 0; y < m_height; y++)
			{
				m_data[y * m_width + x] = c;
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

	void Bitmap::DrawBitmap(I32 x, I32 y, const Bitmap& bitmap)
	{
		for (int i = 0; i < bitmap.m_width; i++)
		{
			for (int j = 0; j < bitmap.m_height; j++)
			{
				SetPixel(x + i, y + j, bitmap.GetPixel(i, j));
			}
		}
	}

	void Bitmap::DrawBitmap(I32 x, I32 y, I32 xb, I32 yb, I32 w, I32 h, const Bitmap& bitmap)
	{
		for (int i = 0; i < w; i++)
		{
			for (int j = 0; j < h; j++)
			{
				SetPixel(x + i, y + j, bitmap.GetPixel(xb + i, yb + j));
			}
		}
	}

	Bitmap Bitmap::Clone() const
	{
		return std::move(Bitmap(m_data.get(), m_width, m_height));
	}

	Bitmap Bitmap::FromFile(const char* filePath)
	{
		tako::U8* buffer = new tako::U8[5242880];
		size_t bytesRead = 0;
		tako::Assets::ReadAssetFile(filePath, buffer, 5242880, bytesRead);
		int width, height, channels;
		stbi_uc* img = stbi_load_from_memory(buffer, bytesRead, &width, &height, &channels, 4);
		Bitmap bitmap((Color*)img, width, height);
		stbi_image_free(img);
		delete[] buffer;

		return std::move(bitmap);
	}

	Bitmap Bitmap::FromFileData(const U8* data, size_t size)
	{;
		int width, height, channels;
		stbi_uc* img = stbi_load_from_memory(data, size, &width, &height, &channels, 4);
		Bitmap bitmap((Color*)img, width, height);
		stbi_image_free(img);

		return std::move(bitmap);
	}
}