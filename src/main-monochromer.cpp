#include "converter.h"
#include "utils.hh"

int main(int argc, const char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: <in-dir> <out-dir> [conversions]\n");
        return 1;
    }

    std::string dir(argv[1]);
    std::filesystem::path out_dir(argv[2]);
    std::vector<std::string> conversions;

    for (auto i = 3u; i < argc; i++)
    {
        auto parts = split_string(argv[i], " ");

        for (auto &cur : parts)
        {
            conversions.emplace_back(cur);
        }
    }

    auto c = std::make_unique<Converter>(dir, conversions);

    auto files = c->updateFiles();
    for (const auto &file : files)
    {
        auto dst_dir = out_dir;
        auto path = dst_dir.append(file.filename().c_str());
        c->storeImageToPng(file, path);
    }

    return 0;
}
