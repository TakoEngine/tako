#pragma once
#include "FileSystem.hpp"

namespace tako::Assets
{
	static std::string GetAssetPath(const char* filePath)
	{
#ifndef TAKO_EMSCRIPTEN
		return FileSystem::GetExecutablePath() + "/Assets" + filePath;
#else
		return filePath;
#endif
	}

	static bool ReadAssetFile(
		const char* filePath,
		U8* buffer,
		size_t bufferSize,
		size_t& bytesRead)
	{
		return FileSystem::ReadFile(GetAssetPath(filePath).c_str(), buffer, bufferSize, bytesRead);
	}

	static size_t GetAssetFileSize(const char* filePath)
	{
		return FileSystem::GetFileSize(GetAssetPath(filePath).c_str());
	}
}
