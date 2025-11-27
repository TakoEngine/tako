module;
#include "Utility.hpp"
#include <RmlUi/Core/FileInterface.h>
#include <vector>
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
			auto file = m_vfs->Open(path);
			if (m_freeList == 0)
			{
				m_openFiles.push_back(file);
				return m_openFiles.size();
			}

			auto index = m_freeList;
			m_freeList = m_openFiles[index - 1].value;
			m_openFiles[index - 1] = file;
			return index;
		}

		void Close(Rml::FileHandle handle) override
		{
			auto file = m_openFiles[handle - 1];
			m_vfs->Close(file);

			m_openFiles[handle - 1].value = m_freeList;
			m_freeList = handle;
		}

		size_t Read(void* buffer, size_t size, Rml::FileHandle handle) override
		{
			auto file = m_openFiles[handle - 1];
			return m_vfs->Read(file, static_cast<U8*>(buffer), size);
		}

		bool Seek(Rml::FileHandle handle, long offset, int origin) override
		{
			auto file = m_openFiles[handle - 1];
			return m_vfs->Seek(file, offset, std::bit_cast<IO::SeekOrigin>(origin));
		}

		size_t Tell(Rml::FileHandle handle) override
		{
			auto file = m_openFiles[handle - 1];
			return m_vfs->Tell(file);
		}
	private:
		VFS* m_vfs;
		std::vector<File> m_openFiles;
		Rml::FileHandle m_freeList = 0;
	};

}