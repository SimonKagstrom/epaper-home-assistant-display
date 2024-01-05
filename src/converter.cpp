#include "converter.h"

Converter::Converter(const std::filesystem::path &directory, const std::vector<std::string> &conversions) :
    m_monitor(IFilesystemMonitor::create(directory)),
    m_conversions(conversions)
{
}

std::vector<std::filesystem::path> Converter::updateFiles()
{
    auto out = std::vector<std::filesystem::path>();

    m_imageData.clear();

    for (auto &cur : m_monitor->getUpdatedFiles())
    {
        m_processor[cur] = IMonochromer::create(cur, m_conversions);
        auto s = m_processor[cur]->process();

        m_imageData[cur] = std::vector<uint8_t>(s.data(), s.data() + s.size());
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
        return std::span<uint8_t>{m_imageData[image].data(), m_imageData[image].size()};
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
