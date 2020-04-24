#pragma once
#include "Utility.hpp"
#include <string_view>
#include <filesystem>
#include <thread>
#include <unordered_map>
#include <atomic>

namespace tako
{
    struct FileChanged
    {
        std::filesystem::path path;
    };

    class FileWatcher
    {
    public:
        FileWatcher(std::string_view watchPath)
        {
            m_thread = std::thread(&FileWatcher::WatchFolder, this, std::string(watchPath));
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

        void WatchFolder(std::string_view path)
        {
            std::unordered_map<std::string, std::filesystem::file_time_type> paths;
            for (auto &file : std::filesystem::recursive_directory_iterator(path)) {
                paths[file.path().string()] = std::filesystem::last_write_time(file);
            }

            while (m_running)
            {
                // Check modified
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
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
                                m_changes.push_back({file.path()});
                                found->second = newWrite;
                                m_changed = true;
                            }
                        }
                        else
                        {
                            //LOG("New file! {}", file.path());
                            paths[file.path().string()] = newWrite;
                            m_changes.push_back({file.path()});
                            m_changed = true;
                        }
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    };
}