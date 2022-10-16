#pragma once

#include <memory>
#include <span>

class IEpaperDisplay
{
public:
    virtual ~IEpaperDisplay() = default;

    virtual void clear() = 0;

    /**
     * Flip the currently drawn page to the screen
     */
    virtual void flip() = 0;

    /**
     * Draw a page
     *
     * @param data the image to draw
     */
    virtual void drawImage(std::span<uint8_t> data) = 0;

    virtual void sleep() = 0;

    static std::unique_ptr<IEpaperDisplay> create();
};