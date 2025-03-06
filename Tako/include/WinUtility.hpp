#pragma once

#include "Utility.hpp"
#include <Windows.h>
#include <functional>
#include <string_view>
#include <iostream>

class WinString
{
public:
	WinString(LPSTR buffer, size_t size);
	WinString(const WinString& other) = delete;
	WinString(WinString&& other);
	WinString& operator=(WinString&& other);
	~WinString();

	constexpr operator std::string_view() const;
private:
	friend std::ostream& operator<<(std::ostream& os, const WinString& str);

	LPSTR buffer;
	size_t size;
};

WinString GetErrorMessage(DWORD err);
WinString GetLastErrorMessage();
