module;
#include "Utility.hpp"
export module Tako.IO;

import Tako.NumberTypes;
import Tako.StringView;

namespace tako::IO
{
    export using FileHandle = void;

    export FileHandle* Open(StringView filePath);
    export void Close(FileHandle* file);

    export size_t Read(FileHandle* file, U8* buffer, size_t size);
    export size_t Write(FileHandle* file, const U8* data, size_t size);

    export enum class SeekOrigin : int
    {
        Start = SEEK_SET,
        Current = SEEK_CUR,
        End = SEEK_END
    };
    export bool Seek(FileHandle* file, long offset, SeekOrigin origin);
    export size_t Tell(FileHandle* file);
}