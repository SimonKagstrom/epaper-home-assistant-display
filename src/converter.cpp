#include "converter.h"

Converter::Converter(const std::filesystem::path &directory, const std::vector<std::string> &conversions) :
    m_monitor(IFilesystemMonitor::create(directory)),
    m_conversions(conversions)
{
}

std::vector<std::filesystem::path> Converter::updateFiles()
{
    auto out = std::vector<std::filesystem::path>();

    for (auto &cur : m_monitor->getUpdatedFiles())
    {
        m_processor[cur] = IMonochromer::create(cur, m_conversions);
        m_imageData[cur] = m_processor[cur]->process();
    }

    for (const auto &[k,v] : m_imageData)
    {
        out.push_back(k);
    }

    return out;
}

std::optional<std::span<uint8_t>> Converter::getImage(const std::filesystem::path &image)
{
    if (m_imageData.contains(image))
    {
        return m_imageData[image];
    }
    return {};
}

void Converter::storeImageToPng(const std::filesystem::path &image, const std::string &destination)
{
    if (m_imageData.contains(image))
    {
        m_processor[image]->storeToPng(destination);
    }
}
