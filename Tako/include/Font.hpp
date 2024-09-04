#pragma once
#include "Utility.hpp"
#include <map>
#include <utility>
#include <string_view>

import Tako.Bitmap;

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
			output.Clear({0, 0, 0, 0});
			int xI = 0;
			int yI = 0;
			for (int i = 0; i < text.length(); i++)
			{
				if (text[i] == '\n' || (charsPerLine > 0 && xI == charsPerLine))
				{
					yI++;
					xI = 0;
					continue;
				}
				auto search = m_charMap.find(text[i]);
				if (search == m_charMap.end())
				{
					continue;
				}
				auto [xb, yb] = search->second;
				auto x = xI * (m_width + xDistance);
				auto y = yI * (m_height + yDistance);
				output.DrawBitmap(x, y, xb, yb, m_width, m_height, m_source);
				xI++;
			}
			return std::move(output);
		}

		std::pair<I32, I32> CalculateDimensions(std::string_view text, int xDistance, int charsPerLine = -1, int yDistance = 1)
		{
			auto len = text.length();
			int lines = 1;
			int maxLineChars = 0;
			{
				int currentLineChars = 0;
				for (int i = 0; i < text.length(); i++)
				{
					if (text[i] == '\n')
					{
						lines++;
						currentLineChars = 0;
					}
					else if (charsPerLine > 0 && currentLineChars >= charsPerLine)
					{
						lines++;
						currentLineChars = 1;
						maxLineChars = std::max(maxLineChars, currentLineChars);
					}
					else
					{
						currentLineChars++;
						maxLineChars = std::max(maxLineChars, currentLineChars);
					}
				}
			}

			return { maxLineChars * (m_width + xDistance) - xDistance, lines * (m_height + yDistance) - yDistance};
		}
	private:
		Bitmap m_source;
		I32 m_width;
		I32 m_height;
		std::map<char, std::pair<I32, I32>> m_charMap;
	};
}
