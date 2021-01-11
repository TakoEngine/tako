#include "Resources.hpp"

namespace tako
{
	template<>
	Texture Resources::Load(std::string path)
	{
		auto search = m_map.find(path);
		if (search != m_map.end())
		{
			return search->second;
		}

		auto bitmap = Bitmap::FromFile(path.c_str());
		auto tex = m_drawer->CreateTexture(bitmap);
		m_map[path] = tex;
		return tex;
	}
}
