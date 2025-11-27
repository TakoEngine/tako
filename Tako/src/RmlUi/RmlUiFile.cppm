module;
#include "Utility.hpp"
#include <RmlUi/Core/FileInterface.h>
export module Tako.RmlUi.File;

import Tako.NumberTypes;
import Tako.VFS;

namespace tako
{
	export class RmlUiFileInterface : public Rml::FileInterface
	{
	public:
		void Init(VFS* vfs)
		{
			m_vfs = vfs;
		}

		Rml::FileHandle Open(const Rml::String& path) override
		{
			return std::bit_cast<Rml::FileHandle>(m_vfs->Open(path));
		}

		void Close(Rml::FileHandle file) override
		{
			m_vfs->Close(std::bit_cast<tako::File>(file));
		}
		size_t Read(void* buffer, size_t size, Rml::FileHandle file) override
		{
			return m_vfs->Read(std::bit_cast<tako::File>(file), static_cast<U8*>(buffer), size);
		}

		bool Seek(Rml::FileHandle file, long offset, int origin) override
		{
			return m_vfs->Seek(std::bit_cast<tako::File>(file), offset, std::bit_cast<IO::SeekOrigin>(origin));
		}

		size_t Tell(Rml::FileHandle file) override
		{
			return m_vfs->Tell(std::bit_cast<tako::File>(file));
		}
	private:
		VFS* m_vfs;
	};

}