#pragma once
#include "Utility.hpp"
#include <string_view>
#include <filesystem>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <atomic>

import Tako.VFS;

namespace tako
{
	struct FileChanged
	{
		std::filesystem::path path;
		std::filesystem::path mountPath;
	};

	class FileWatcher
	{
	public:
		FileWatcher(VFS* vfs)
		{
			m_thread = std::thread(&FileWatcher::WatchFolder, this, vfs);
		}

		~FileWatcher()
		{
			m_running = false;
			m_thread.join();
		}

		std::vector<FileChanged> Poll()
		{
			std::vector<FileChanged> result;
			if (!m_changed)
			{
				return result;
			}
			std::lock_guard<std::mutex> lock(m_mutex);
			std::swap(result, m_changes);
			m_changed = false;
			return result;
		}

	private:
		std::atomic<bool> m_running = true;
		std::atomic<bool> m_changed = true;
		std::thread m_thread;
		std::mutex m_mutex;
		std::vector<FileChanged> m_changes;

		void WatchFolder(VFS* vfs)
		{
			std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> paths;

			for (auto& path : vfs->GetMountPaths())
			{
				for (auto &file : std::filesystem::recursive_directory_iterator(path))
				{
					paths[file.path()] = std::filesystem::last_write_time(file);
				}
			}

			while (m_running)
			{
				// Check modified
				{
					std::vector<FileChanged> changes;

					for (auto& path : vfs->GetMountPaths())
					{
						for (auto &file : std::filesystem::recursive_directory_iterator(path))
						{
							if (!std::filesystem::exists(file.path())) {
								continue;
							}
							auto newWrite = std::filesystem::last_write_time(file);

							auto found = paths.find(file.path());
							if (found != paths.end())
							{
								auto lastWrite = found->second;
								if (lastWrite != newWrite) {
									//LOG("File changed! {}", file.path());
									changes.push_back({file.path(), path});
									found->second = newWrite;

								}
							}
							else
							{
								//LOG("New file! {}", file.path());
								paths[file.path()] = newWrite;
								changes.push_back({file.path(), path});
							}
						}
					}

					if (changes.size() > 0)
					{
						std::lock_guard<std::mutex> lock(m_mutex);
						m_changed = true;
						m_changes.insert(m_changes.end(), changes.begin(), changes.end());
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		}
	};
}
