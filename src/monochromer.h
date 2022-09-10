#pragma once

#include <string>
#include <vector>
#include <span>

#include <memory>

class IMonochromer
{
public:
    virtual ~IMonochromer() = default;

    virtual std::span<uint8_t> process() = 0;

    static std::unique_ptr<IMonochromer> create(const std::string &filename, const std::vector<std::string> &conversions);
};
