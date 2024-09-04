module;
#include <string>
export module Tako.FileSystem;

export import :OS; // Ideally the os implementation would just be implementation modules, but they are not supported yet
import Tako.NumberTypes;

/*
export namespace tako::FileSystem
{
	bool ReadFile(
		const char* filePath,
		U8* buffer,
		size_t bufferSize,
		size_t& bytesRead);

	size_t GetFileSize(const char* filePath);

	std::string GetExecutablePath();
}
*/

export namespace tako::FileSystem
{
	std::string ReadText(const char* filePath)
	{
		size_t fileSize = GetFileSize(filePath);
		std::string str(fileSize, '\0');
		size_t bytesRead = 0;
		bool readSuccess = ReadFile(filePath, (U8*)str.data(), str.size(), bytesRead);
		return str;
	}
}