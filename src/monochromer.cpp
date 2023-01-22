#include "monochromer.h"

#include <functional>
#include <map>
#define cimg_use_png 1
#include <CImg.h>

#include "utils.hh"

using namespace cimg_library; 

namespace
{

class Color
{
public:
    Color(int _r, int _g, int _b) :
        r(_r),
        g(_g),
        b(_b)
    {
    }

    ~Color() = default;

    static Color atImage(const CImg<unsigned char> &img, int col, int row)
    {
        auto r = img(col,row,0, 0);
        auto g = img(col,row,0, 1);
        auto b = img(col,row,0, 2);

        return Color(r,g,b);
    }

    bool operator<=>(const Color &other) const = default;

    uint32_t value() const
    {
        return (r << 16) | (g << 8) | b;
    }

    std::array<int, 3> toArray() const
    {
        return {r,g,b};
    }

    int r;
    int g;
    int b;
};

}

class Monochromer : public IMonochromer
{
public:
    Monochromer(const std::string &filename, const std::vector<std::string> &conversions);

    std::span<uint8_t> process() final;

    void storeToPng(const std::string &destination) final;

private:
    CImg<unsigned char> m_inputImage;
    std::vector<uint8_t> m_processedImage;

    static Color convertVertical(int x, int /*y*/)
    {
        if (x % 6 < 5)
        {
            return {0xff, 0xff, 0xff};
        }

        return {0, 0, 0};
    }

    static Color convertHorizontal(int /*x*/, int y)
    {
        if (y % 6 < 5)
        {
            return {0xff, 0xff, 0xff};
        }

        return {0, 0, 0};
    }

    static Color convertRightSlant(int x, int y)
    {
        if ((x + y) % 6 < 5)
        {
            return {0xff, 0xff, 0xff};
        }

        return {0, 0, 0};
    }

    static Color convertLeftSlant(int x, int y)
    {
        if ((x - y) % 6 < 5)
        {
            return {0xff, 0xff, 0xff};
        }

        return {0, 0, 0};
    }

    static Color convertSolid(int x, int y)
    {
        return {0, 0, 0};
    }

    static Color convertDotted(int x, int y)
    {
        if (x % 8 == 0 && y % 8 == 0)
        {
            return {0, 0, 0};
        }

        return {0xff, 0xff, 0xff};
    }

    std::map<unsigned long, std::function<Color(int x, int y)>> m_conversions;
};

Monochromer::Monochromer(const std::string &filename, const std::vector<std::string> &conversions) :
    m_inputImage(CImg(filename.c_str()))
{
    m_processedImage.reserve((m_inputImage.width() * m_inputImage.height()) / 8);

    for (auto &conv : conversions)
    {
        auto v = split_string(conv, "=");

        if (v.size() != 2)
        {
            continue;
        }

        auto k = v[1];

        if (k.size() < 2)
        {
            continue;
        }

        auto even_out = false;
        if (v[0][0] == '~')
        {
            v[0] = v[0].substr(1, v[0].size());
            even_out = true;
        }

        auto value = string_to_integer(v[0],16);

        if (k == "vertical")
        {
            m_conversions[value] = convertVertical;
        }
        if (k == "horizontal")
        {
            m_conversions[value] = convertHorizontal;
        }
        if (k == "rightslant")
        {
            m_conversions[value] = convertRightSlant;
        }
        if (k == "leftslant")
        {
            m_conversions[value] = convertLeftSlant;
        }
        if (k == "solid")
        {
            m_conversions[value] = convertSolid;
        }
        if (k == "dotted")
        {
            m_conversions[value] = convertDotted;
        }

        if (even_out)
        {
            for (auto i = 1u; i <= 8; i++)
            {
                auto next = value + 0x010101 * i;
                auto prev = value - 0x010101 * i;

                m_conversions[next] = m_conversions[value];
                m_conversions[prev] = m_conversions[value];
            }
        }
    }
}

std::span<uint8_t> Monochromer::process()
{
    constexpr auto blackLimit = static_cast<int>(255 * 3 * 0.94);
    const Color white(255,255,255);
    const Color black(0, 0, 0);

    m_processedImage.clear();
    for (auto r = m_inputImage.height() - 480u; r < m_inputImage.height(); r++)
    {
        uint8_t curByte = 0;
        int bit = 0;

        for (auto c = 0u; c < m_inputImage.width(); c++)
        {
            auto curColor = Color::atImage(m_inputImage, c, r);

            auto color = white;
            if (m_conversions.contains(curColor.value()))
            {
                color = m_conversions[curColor.value()](c, r);
            }
            else if (curColor.r + curColor.g + curColor.b < blackLimit)
            {
                color = black;
            }

            if (color == white)
            {
                curByte |= (1 << (7-bit));
            }

            if (++bit >= 8)
            {
                m_processedImage.push_back(curByte);
                bit = 0;
                curByte = 0;
            }
        }
        if (bit != 0)
        {
            m_processedImage.push_back(curByte);
        }
    }

    return {m_processedImage.data(), m_processedImage.size()};
}

void Monochromer::storeToPng(const std::string &path)
{
    printf("%s\n", path.c_str());
    constexpr auto blackLimit = static_cast<int>(255 * 3 * 0.94);
    const Color white(255,255,255);
    const Color black(0, 0, 0);

    auto out = CImg<unsigned char>(m_inputImage.width(), 480, 1, 3, 0);

    for (auto r = m_inputImage.height() - 480u; r < m_inputImage.height(); r++)
    {
        uint8_t curByte = 0;
        int bit = 0;

        for (auto c = 0u; c < m_inputImage.width(); c++)
        {
            auto curColor = Color::atImage(m_inputImage, c, r);

            auto color = white;
            if (m_conversions.contains(curColor.value()))
            {
                color = m_conversions[curColor.value()](c, r);
            }
            else if (curColor.r + curColor.g + curColor.b < blackLimit)
            {
                color = black;
            }

            out.draw_point(c, r, color.toArray().data());
        }
    }
    out.save(path.c_str());
}


std::unique_ptr<IMonochromer> IMonochromer::create(const std::string &filename, const std::vector<std::string> &conversions)
{
    return std::make_unique<Monochromer>(filename, conversions);
}
