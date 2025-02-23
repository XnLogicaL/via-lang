// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#pragma once

#include "common.h"
#include <format>
#include <functional>
#include <string>
#include <vector>

namespace via::utils {

template<typename T>
std::string format_vector(const std::vector<T> &, std::function<std::string(const T &)>);

} // namespace via::utils
