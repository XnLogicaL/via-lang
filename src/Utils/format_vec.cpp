#include "format_vec.h"

template <typename T>
std::string format_vector(const std::vector<T>& vec, std::function<std::string(const T)> func)
{
    std::string result = "[";
    bool first         = true;

    for (const auto& item : vec)
    {
        if (!first)
        {
            result += ", ";
        }

        result += std::format("{}", func(item));
        first = false;
    }

    result += "]";
    return result;
}