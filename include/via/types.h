// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_TYPES_H_
#define VIA_TYPES_H_

#include <fmt/format.h>
#include <cassert>
#include <deque>
#include <expected>
#include <filesystem>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace via
{

namespace fs = std::filesystem;

using std::literals::operator""s;
using std::literals::operator""sv;

template <typename T>
using Ref = std::reference_wrapper<T>;

using String = std::string;
using StringView = std::string_view;

template <typename Sig>
using Function = std::function<Sig>;

template <typename... Args>
using Fmt = fmt::format_string<Args...>;

template <typename K,
          typename V,
          typename Hash = std::hash<K>,
          typename Eq = std::equal_to<K>>
using Map = std::unordered_map<K, V, Hash, Eq>;

template <typename T>
using Set = std::unordered_set<T>;

template <typename T, const size_t Size>
using Array = std::array<T, Size>;

template <typename T>
using Deque = std::deque<T>;

template <typename T>
using Vec = std::vector<T>;

template <typename T>
using Box = std::unique_ptr<T>;

template <typename T>
using Atomic = std::atomic<T>;

template <typename T>
using Rc = std::shared_ptr<T>;

template <typename T>
using Arc = Atomic<Rc<T>>;

template <typename T, typename U>
using Pair = std::pair<T, U>;

template <typename T, typename E>
using Result = std::expected<T, E>;

template <typename... Ts>
using Variant = std::variant<Ts...>;
using monostate = std::monostate;

template <typename T>
using Optional = std::optional<T>;
inline constexpr auto nullopt = std::nullopt;

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

using f32 = float;
using f64 = double;

template <typename T>
using Limits = std::numeric_limits<T>;

}  // namespace via

#endif
