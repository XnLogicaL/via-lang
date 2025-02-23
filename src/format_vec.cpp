// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#include "format_vec.h"

namespace via::utils {

template<typename T>
std::string format_vector(const std::vector<T> &vec, std::function<std::string(const T &)> to_str)
{
    std::string str;

    for (const T &val : vec) {
        str += to_str(val) + ", ";
    }

    if (str.ends_with(' ')) {
        str += "\b\b";
    }

    return std::format("{}{}{}", '{', str, '}');
}

} // namespace via::utils
