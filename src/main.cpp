#include <iostream>
#include "Windows.h"
#include "Window.hpp"

int main()
{
    tako::Window window;

    while (!window.ShouldExit())
    {
        window.Poll();
        Sleep(16);
    }
    
    return 0;
}