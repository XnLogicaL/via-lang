#pragma once

#include "common.h"
#include <format>
#include <functional>
#include <string>
#include <vector>

namespace via
{

template<typename T>
VIA_FORCEINLINE std::string format_vector(const std::vector<T> &vec, std::function<std::string(const T)> func)
{
    std::string str;

    for (const T &val : vec)
        str += func(val) + ", ";

    if (str.ends_with(' '))
        str += "\b\b";

    return std::format("{{{}}}", str);
}

} // namespace via
