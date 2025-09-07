/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace via
{

namespace fs = std::filesystem;

using std::literals::operator""s;
using std::literals::operator""sv;

template <typename K,
          typename V,
          typename Hash = std::hash<K>,
          typename Eq = std::equal_to<K>>
using Map = std::unordered_map<K, V, Hash, Eq>;

template <typename T>
using Vec = std::vector<T>;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using usize = size_t;
using uptr = uintptr_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using isize = ssize_t;
using iptr = intptr_t;

using f32 = float;
using f64 = double;

}  // namespace via
