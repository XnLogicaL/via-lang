#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <unordered_map>
#include <map>
#include <format>
#include <stdexcept>
#include <functional>
#include <memory>
#include <stack>
#include <utility>
#include <bitset>
#include <cstdlib>
#include <cstddef>
#include <cstdbool>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <cstddef>
#include <cctype>
#include <cassert>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <thread>

#ifndef VIA_FORMATVEC
#define VIA_FORMATVEC

template <typename T>
std::string format_vector(const std::vector<T>& vec, std::function<std::string(const T)> func)
{
    std::string result = "[";
    bool first = true;

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

#endif