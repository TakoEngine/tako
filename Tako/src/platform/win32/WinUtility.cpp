#include "WinUtility.hpp"


WinString::WinString(LPSTR buffer, size_t size)
{
	this->buffer = buffer;
	this->size = size;
}

WinString::WinString(WinString&& other)
{
	buffer = other.buffer;
	size = other.size;
	other.buffer = nullptr;
}

WinString& WinString::operator=(WinString&& other)
{
	if (this == &other) return *this;
	if (buffer != nullptr) LocalFree(buffer);

	buffer = other.buffer;
	size = other.size;
	other.buffer = nullptr;
}

WinString::~WinString()
{
	if (buffer == nullptr) return;
	LocalFree(buffer);
}

constexpr WinString::operator std::string_view() const
{
	return { buffer, size };
}

std::ostream& operator<<(std::ostream& os, const WinString& str)
{
	os << str.operator std::string_view();
	return os;
}

WinString GetErrorMessage(DWORD err)
{
	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)& messageBuffer, 0, NULL);

	return { messageBuffer, size };
}

WinString GetLastErrorMessage()
{
	return GetErrorMessage(GetLastError());
}
