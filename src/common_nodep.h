// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#if not defined(__GNUC__) && not defined(__clang__)
    #pragma error(Unsupported compiler.Please use g++ or clang++.)
#endif

#pragma once

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
#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include <stacktrace>
#include <shared_mutex>
#include <limits>
#include <typeindex>

namespace via {

using U8  = unsigned char;
using U16 = unsigned short;
using U32 = unsigned int;
using U64 = unsigned long long;

using I8  = char;
using I16 = short;
using I32 = int;
using I64 = long long;

using F32 = float;
using F64 = double;

using SIZE = unsigned long long;

} // namespace via
