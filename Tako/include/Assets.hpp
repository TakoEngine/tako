#pragma once
#include "FileSystem.hpp"

namespace tako::Assets
{
	bool ReadAssetFile(
		const char* filePath,
		U8* buffer,
		size_t bufferSize,
		size_t& bytesRead)
	{
		FileSystem::GetExecutablePath();
		auto path = std::string("./Assets") + filePath;
		return FileSystem::ReadFile(path.c_str(), buffer, bufferSize, bytesRead);
	}

	size_t GetAssetFileSize(const char* filePath)
	{
		auto path = std::string("./Assets") + filePath;
		return FileSystem::GetFileSize(path.c_str());
	}
}
