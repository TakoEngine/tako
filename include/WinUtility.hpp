#pragma once
#include "Utility.hpp"
#include <Windows.h>
#include <functional>

class WinString
{
public:
    WinString(LPSTR buffer, size_t size)
    {
        this->buffer = buffer;
        this->size = size;
    }

    WinString(const WinString& other) = delete;

    WinString(WinString&& other)
    {
        buffer = other.buffer;
        size = other.size;
        other.buffer = nullptr;
    }

    WinString& operator=(WinString&& other)
    {
        if (this == &other) return *this;
        if (buffer != nullptr) LocalFree(buffer);

        buffer = other.buffer;
        size = other.size;
        other.buffer = nullptr;
    }

    ~WinString()
    {
        if (buffer == nullptr) return;
        LocalFree(buffer);
    }

    constexpr operator std::string_view() const
    {
        return { buffer, size };
    }

private:
    friend std::ostream& operator<<(std::ostream& os, const WinString& str)
    {
        os << str.operator std::string_view();
        return os;
    }

    LPSTR buffer;
    size_t size;
};

WinString GetErrorMessage(DWORD err)
{
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    return { messageBuffer, size };
}

WinString GetLastErrorMessage()
{
    return GetErrorMessage(GetLastError());
}