#pragma once
#include "NumberTypes.hpp"

namespace tako::FileSystem
{
    bool ReadFile(const char* filePath,
                  U8* buffer,
                  size_t bufferSize,
                  size_t& bytesRead);

    size_t GetFileSize(const char* filePath);
}