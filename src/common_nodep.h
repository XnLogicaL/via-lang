// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#if !defined(__GNUC__) && !defined(__clang__)
#error Unsupported compiler: as of via 0.20.x, only GNU g++ and clang++ are supported.
#endif

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

#include "api_config.h"

VIA_NAMESPACE_BEGIN

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using i8  = char;
using i16 = short;
using i32 = int;
using i64 = long long;

using f32 = float;
using f64 = double;

VIA_NAMESPACE_END

#endif
