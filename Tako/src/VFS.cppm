module;
#include "Utility.hpp"
#include <vector>
#include <memory>
#include <span>
export module Tako.VFS;

import Tako.NumberTypes;
import Tako.StringView;
import Tako.HandleVec;
export import Tako.IO;

namespace tako
{
    export struct File
    {
        U64 value;
    };

    struct FileHandleData
    {
        IO::FileHandle* handle;
    };

    export class VFS
    {
    public:
        File Open(StringView path)
        {
            auto handle = OpenFilesystem(path);
            if (handle)
            {
                FileHandleData entry;
                entry.handle = handle;
                return m_handles.Insert(std::move(entry));
            }

            return {0}; // Null handle
        }

        void Close(File file)
        {
            auto& entry = m_handles[file];
            IO::Close(entry.handle);
            m_handles.Remove(file);
        }

        size_t Read(File file, U8* buffer, size_t size)
        {
            auto& entry = m_handles[file];
            return IO::Read(entry.handle, buffer, size);
        }

        size_t Write(File file, const U8* data, size_t size)
        {
            auto& entry = m_handles[file];
            return IO::Write(entry.handle, data, size);
        }

        bool Seek(File file, long offset, IO::SeekOrigin origin)
        {
            auto& entry = m_handles[file];
            return IO::Seek(entry.handle, offset, origin);
        }

        size_t Tell(File file)
        {
            auto& entry = m_handles[file];
            return IO::Tell(entry.handle);
        }

        std::string LoadText(StringView path)
        {
            auto file = OpenFilesystem(path);
            ASSERT(file);
            auto fileSize = GetSizeFilesystem(file);
            IO::Seek(file, 0, IO::SeekOrigin::Start);
            std::string str(fileSize, '\0');
            IO::Read(file, reinterpret_cast<U8*>(str.data()), str.size());
            IO::Close(file);
            return str;
        }

        std::vector<U8> LoadFile(StringView path) const
        {
            auto file = OpenFilesystem(path);
            ASSERT(file);
            auto fileSize = GetSizeFilesystem(file);
            IO::Seek(file, 0, IO::SeekOrigin::Start);
            std::vector<U8> buffer(fileSize);
            IO::Read(file, reinterpret_cast<U8*>(buffer.data()), buffer.size());
            IO::Close(file);
            return buffer;
        }

        void AddMountPath(StringView path)
        {
            m_mountPaths.insert(m_mountPaths.begin(), path.ToString());
        }

		std::span<std::string> GetMountPaths()
		{
			return m_mountPaths;
		}
    private:
        std::vector<std::string> m_mountPaths;
        tako::HandleVec<File, FileHandleData> m_handles;

        IO::FileHandle* OpenFilesystem(StringView path) const
        {
            ASSERT(m_mountPaths.size() > 0);
            std::string adjustedPath;
            for (auto& mountPath : m_mountPaths)
            {
                adjustedPath = mountPath;
                adjustedPath.append(path);
                auto handle = IO::Open(adjustedPath);
                if (handle)
                {
                    return handle;
                }
            }

            return nullptr;
        }

        static size_t GetSizeFilesystem(IO::FileHandle* file)
        {
            IO::Seek(file, 0, IO::SeekOrigin::End);
            return IO::Tell(file);
        }
    };
}