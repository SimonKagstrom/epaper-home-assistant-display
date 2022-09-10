#include "filesystem_monitor.h"

#include <set>
#include <map>

#include <fmt/format.h>

class FilesystemMonitor : public IFilesystemMonitor
{
public:
    explicit FilesystemMonitor(const std::filesystem::path &directory) :
        m_directory(directory)
    {
    }

    std::set<std::filesystem::path> getUpdatedFiles() final
    {
        std::set<std::filesystem::path> out;

        findFiles();

        for (auto &cur : m_files)
        {
            if (!m_modificationTime.contains(cur))
            {
                out.insert(cur);
                continue;
            }

            if (std::filesystem::last_write_time(cur) > m_modificationTime[cur])
            {
                printf("Found file %s\n", cur.c_str());
                out.insert(cur);
            }
        }

        return out;
    }

private:
    void findFiles()
    {
        m_files.clear();

        auto i = 0u;
        while(true)
        {
            auto cur = std::filesystem::path(m_directory).append(fmt::format("{}.png", i));
            if (!std::filesystem::exists(cur))
            {
                break;
            }

            m_files.insert(cur);

            i++;
        }
    }

    std::filesystem::path m_directory;
    std::set<std::filesystem::path> m_files;

    std::map<std::filesystem::path, std::filesystem::file_time_type> m_modificationTime;
};

std::unique_ptr<IFilesystemMonitor> IFilesystemMonitor::create(const std::filesystem::path &path)
{
    return std::make_unique<FilesystemMonitor>(path);
}
