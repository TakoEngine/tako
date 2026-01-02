module;
#include "Utility.hpp"
#include <cstdio>
module Tako.IO;

namespace tako::IO
{
	const char* ModeToStr(FileOpenMode mode)
	{
		switch (mode)
		{
			case FileOpenMode::Read: return "rb";
			case FileOpenMode::Write: return "wb";
		}
	}

    FileHandle* Open(StringView filePath, FileOpenMode mode)
    {
        CStringBuffer pathBuffer(filePath);
        return fopen(pathBuffer.c_str(), ModeToStr(mode));
    }

    void Close(FileHandle* file)
    {
        fclose(static_cast<FILE*>(file));
    }

    size_t Read(FileHandle* file, U8* buffer, size_t size)
    {
        return fread(buffer, sizeof(U8), size, static_cast<FILE*>(file));
    }

    size_t Write(FileHandle* file, const U8* data, size_t size)
    {
        return fwrite(data, sizeof(U8), size, static_cast<FILE*>(file));
    }

    bool Seek(FileHandle* file, long offset, SeekOrigin origin)
    {
        return fseek(static_cast<FILE*>(file), offset, static_cast<int>(origin));
    }

    size_t Tell(FileHandle* file)
    {
        return ftell(static_cast<FILE*>(file));
    }
}