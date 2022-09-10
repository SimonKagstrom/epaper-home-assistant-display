#pragma once

#include <memory>
#include <span>

class IEpaperDisplay
{
public:
    virtual ~IEpaperDisplay() = default;

    virtual void clear() = 0;

    virtual void drawImage(std::span<uint8_t> data) = 0;

    virtual void sleep() = 0;

    static std::unique_ptr<IEpaperDisplay> create();
};