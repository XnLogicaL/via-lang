// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_COMMON_NODEP_H
#define _VIA_COMMON_NODEP_H

// C++ std imports
#include <bitset>
#include <cassert>
#include <cctype>
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <stack>
#include <stacktrace>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include <expected>

#include "common-macros.h"

VIA_NAMESPACE_BEGIN

using float32_t  = float;
using float64_t  = double;
using float128_t = long double;

VIA_NAMESPACE_END

#endif
