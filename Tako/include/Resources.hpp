#pragma once
#include "PixelArtDrawer.hpp"
#include <unordered_map>
#include <string>
#include <string_view>

namespace tako
{
    class Resources
    {
    public:
        Resources(PixelArtDrawer* drawer)
        {
            m_drawer = drawer;
        }

        template<typename T>
        T* Load(std::string path);
    private:
        std::unordered_map<std::string, Texture*> m_map;
        PixelArtDrawer* m_drawer;
    };
}