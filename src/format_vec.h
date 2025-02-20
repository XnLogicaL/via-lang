// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
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
