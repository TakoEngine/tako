#pragma once
#include "NumberTypes.hpp"
#include <string>

namespace tako::FileSystem
{
	bool ReadFile(
		const char* filePath,
		U8* buffer,
		size_t bufferSize,
		size_t& bytesRead);

	size_t GetFileSize(const char* filePath);

	std::string GetExecutablePath();

	static std::string ReadText(const char* filePath)
	{
		size_t fileSize = FileSystem::GetFileSize(filePath);
		std::string str(fileSize, '\0');
		size_t bytesRead = 0;
		bool readSuccess = FileSystem::ReadFile(filePath, (U8*)str.data(), str.size(), bytesRead);
		return str;
	}
}
