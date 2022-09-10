#pragma once

#include <sstream>

// http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
static std::vector<std::string> &split(const std::string &s, char delim,
                std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }

    return elems;
}

static std::vector<std::string> split_string(const std::string &s, const char *delims)
{
    std::vector<std::string> elems;
    split(s, *delims, elems);

    return elems;
}

static unsigned long string_to_integer(const std::string &str, unsigned base)
{
    size_t pos;

    return stoul(str, &pos, base);
}
