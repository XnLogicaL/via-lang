// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CONFIG_H_
#define VIA_CONFIG_H_

// Check if libstacktrace is available.
#if __has_include(<stacktrace>) && __cplusplus >= 202302L
#include <stacktrace>
#define VIA_HASSTACKTRACE 1
#else
#define VIA_HASSTACKTRACE 0
#endif

#define VIA_VERSION "0.0.2"
#define VIA_WORDSIZE sizeof(void*)

#define VIA_BUG(msg)                                \
  assert(false &&                                   \
         "internal bug (please create an issue at " \
         "https://github.com/XnLogicaL/via-lang): " msg)

#define VIA_TODO(msg) assert(false && "TODO: " msg);
#define VIA_UNIMPLEMENTED(msg) assert(false && "unimplemented: " msg);

#define VIA_NOCOPY(target)                   \
  target& operator=(const target&) = delete; \
  target(const target&) = delete;

#define VIA_IMPLCOPY(target)        \
  target& operator=(const target&); \
  target(const target&);

#define VIA_IMPLCOPY_CONSTEXPR(target)        \
  constexpr target& operator=(const target&); \
  constexpr target(const target&);

#define VIA_NOMOVE(target)              \
  target& operator=(target&&) = delete; \
  target(target&&) = delete;

#define VIA_IMPLMOVE(target)   \
  target& operator=(target&&); \
  target(target&&);

#if VIA_HASSTACKTRACE == 1
#define VIA_STACKTRACE std::stacktrace::current()
#else
#define VIA_STACKTRACE ""
#endif

#include <fmt/core.h>
#include <fmt/format.h>
#include <mimalloc.h>
#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace via {

using String = std::string;
using StringView = std::string_view;

template <typename Sig>
using Function = std::function<Sig>;

template <typename... Args>
using Fmt = fmt::format_string<Args...>;

template <typename K, typename V>
using Map = std::unordered_map<K, V>;

template <typename T>
using Set = std::unordered_set<T>;

template <typename T, const size_t Size>
using Array = std::array<T, Size>;

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

template <typename T>
using Optional = std::optional<T>;
inline constexpr auto nullopt = std::nullopt;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using usize = size_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

namespace detail {

template <typename T>
concept has_to_string = requires(T t) {
                          { t.to_string() } -> std::same_as<String>;
                        };

}  // namespace detail

}  // namespace via

template <typename T>
struct fmt::
    formatter<T, char, std::enable_if_t<via::detail::has_to_string<T>>> {
  constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const T& t, FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "{}", t.to_string());
  }
};

#endif
