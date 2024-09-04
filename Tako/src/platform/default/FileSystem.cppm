module;
#include "Utility.hpp"
#include "NumberTypes.hpp"
#include <filesystem>
#include <fstream>
#include <string>
export module Tako.FileSystem:OS;

import Tako.Application;

export namespace tako::FileSystem
{
	bool ReadFile(const char* filePath,
					U8* buffer,
					size_t bufferSize,
					size_t& bytesRead)
	{
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open())
		{
			LOG_ERR("Cant open file {}", filePath);
			return false;
		}

		file.seekg(0, std::ios::end);
		auto size = file.tellg();
		auto toRead = std::min<std::size_t>(size, bufferSize);
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(buffer), toRead);
		bytesRead = toRead;
		file.close();

		return true;
	}

	size_t GetFileSize(const char* filePath)
	{
		return std::filesystem::file_size(filePath);
	}

	std::string GetExecutablePath()
	{
		static auto path = std::filesystem::weakly_canonical(Application::argv[0]).parent_path().string();
		return path;
	}
}

