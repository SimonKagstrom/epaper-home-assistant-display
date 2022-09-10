#pragma once

#include <memory>
#include <filesystem>
#include <set>

class IFilesystemMonitor
{
public:
    virtual ~IFilesystemMonitor() = default;

    virtual std::set<std::filesystem::path> getUpdatedFiles() = 0;

    static std::unique_ptr<IFilesystemMonitor> create(const std::filesystem::path &path);
};
