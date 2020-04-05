#include "FileSystem.hpp"
#include "Utility.hpp"
#include <filesystem>
#include <fstream>

namespace tako::FileSystem
{
    bool ReadFile(const char* filePath,
                  U8* buffer,
                  size_t bufferSize,
                  size_t& bytesRead)
    {
        std::string path("./Assets");
        path.append(filePath);
        std::ifstream file(path);
        if (!file.is_open())
        {
            LOG_ERR("Cant open file {}", path);
            return false;
        }

        file.seekg(0, std::ios::end);
        auto size = file.tellg();
        auto toRead = std::min<std::size_t>(size, bufferSize);
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(buffer), toRead);
        bytesRead = toRead;
        file.close();
    }

    size_t GetFileSize(const char* filePath)
    {
        return std::filesystem::file_size(filePath);
    }
}

