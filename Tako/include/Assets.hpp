#pragma once
#include "FileSystem.hpp"

namespace tako::Assets
{
	std::string GetAssetPath(const char* filePath)
	{
		return FileSystem::GetExecutablePath() + "/Assets" + filePath;
	}

	bool ReadAssetFile(
		const char* filePath,
		U8* buffer,
		size_t bufferSize,
		size_t& bytesRead)
	{
		return FileSystem::ReadFile(GetAssetPath(filePath).c_str(), buffer, bufferSize, bytesRead);
	}

	size_t GetAssetFileSize(const char* filePath)
	{
		return FileSystem::GetFileSize(GetAssetPath(filePath).c_str());
	}
}
