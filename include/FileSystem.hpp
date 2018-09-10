#pragma once

namespace tako::FileSystem
{
    bool ReadFile(const char* filePath,
                  char* buffer,
                  size_t bufferSize,
                  size_t& bytesRead);
}