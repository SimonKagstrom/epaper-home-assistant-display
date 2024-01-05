#pragma once

#include "monochromer.h"
#include "filesystem_monitor.h"

#include <optional>
#include <map>

class Converter
{
public:
    explicit Converter(const std::filesystem::path &directory, const std::vector<std::string> &conversions);

    std::vector<std::filesystem::path> updateFiles();

    std::optional<std::span<uint8_t>> getImage(const std::filesystem::path &image);

    void storeImageToPng(const std::filesystem::path &image, const std::string &destination);

private:
    std::unique_ptr<IFilesystemMonitor> m_monitor;
    std::map<std::filesystem::path, std::unique_ptr<IMonochromer>> m_processor;
    std::map<std::filesystem::path, std::vector<uint8_t>> m_imageData;

    std::vector<std::string> m_conversions;
};
