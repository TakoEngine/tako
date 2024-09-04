module;
#include <Windows.h>
#include "WinUtility.hpp"
#include <optional>
export module Tako.FileSystem:OS;

std::optional<HANDLE> GetFileHandle(const char* filePath)
{
	HANDLE fileHandle = ::CreateFile(filePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		LOG_ERR("Can't open file {}: {}", filePath, GetLastErrorMessage());
		return {};
	}

	return fileHandle;
}

export namespace tako::FileSystem
{
	bool ReadFile(const char* filePath, U8* buffer, size_t bufferSize, size_t& bytesRead)
	{
		auto fileHandle = GetFileHandle(filePath);
		if (!fileHandle)
		{
			return false;
		}

		DWORD bytes;
		BOOL result = ::ReadFile(*fileHandle, buffer, bufferSize, &bytes, NULL);

		if (result == TRUE)
		{
			bytesRead = bytes;
			CloseHandle(*fileHandle);
			return true;
		}

		LOG_ERR("Error reading file {}: {}", filePath, GetLastErrorMessage());
		CloseHandle(*fileHandle);
		return false;
	}

	size_t GetFileSize(const char* filePath)
	{
		auto fileHandle = GetFileHandle(filePath);
		ASSERT(fileHandle);

		LARGE_INTEGER size;
		GetFileSizeEx(fileHandle.value(), &size);

		return size.QuadPart;
	}

	std::string GetExecutablePath()
	{
		TCHAR buffer[1024];
		auto readSize = GetModuleFileName(NULL, &buffer[0], 1024);
		if (readSize < 1024)
		{
			TCHAR* b = &buffer[readSize - 1];
			while (b > buffer)
			{
				if (*b == '\\')
				{
					*b = '\0';
					break;
				}
				b--;
			}
			return { buffer };
		}
		ASSERT(false); //TODO
	}
}

