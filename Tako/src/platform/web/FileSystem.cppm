module;
#include "Utility.hpp"
#include <cstdio>
export module Tako.FileSystem:OS;

import Tako.NumberTypes;

export namespace tako::FileSystem
{
	bool ReadFile(const char* filePath, U8* buffer, size_t bufferSize, size_t& bytesRead)
	{
		FILE* file = fopen(filePath, "rb");
		if (!file)
		{
			LOG_ERR("Couldn't read {}", filePath);
			return false;
		}
		fseek(file, 0, SEEK_END);
		auto size = bytesRead = ftell(file);
		rewind(file);
		bytesRead = fread(reinterpret_cast<void*>(buffer), 1, size, file);
		return true;
	}

	size_t GetFileSize(const char* filePath)
	{
		FILE* file = fopen(filePath, "rb");
		if (!file)
		{
			LOG_ERR("Couldn't read {}", filePath);
			return false;
		}
		fseek(file, 0, SEEK_END);
		return ftell(file);
	}

	std::string GetExecutablePath()
	{
		return "";
	}
}

