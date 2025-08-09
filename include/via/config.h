// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CONFIG_H_
#define VIA_CONFIG_H_

#if defined(_WIN32) || defined(_WIN64)
#define VIA_PLATFORM_WINDOWS
#endif

#if defined(__linux__)
#ifdef __ANDROID__
#define VIA_PLATFORM_ANDROID
#else
#define VIA_PLATFORM_LINUX
#endif
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#define VIA_PLATFORM_IOS
#else
#define VIA_PLATFORM_MACOS
#endif
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__bsdi__) || defined(__DragonFly__)
#define VIA_PLATFORM_BSD
#endif

#if defined(__EMSCRIPTEN__)
#define VIA_PLATFORM_WEB
#endif

#if defined(VIA_PLATFORM_LINUX) || defined(VIA_PLATFORM_MACOS) || \
    defined(VIA_PLATFORM_BSD)
#define VIA_PLATFORM_POSIX
#endif

#if defined(VIA_PLATFORM_POSIX) || defined(VIA_PLATFORM_ANDROID)
#define VIA_PLATFORM_UNIX
#endif

#ifdef VIA_PLATFORM_WINDOWS
#define VIA_EXPORT __declspec(dllexport)
#else
#define VIA_EXPORT
#endif

#ifdef __GNUC__
#ifdef __clang__
#define VIA_COMPILER_CLANG
#else
#define VIA_COMPILER_GCC
#endif
#endif

#ifdef _MSC_VER
#define VIA_COMPILER_MSVC
#endif

#define VIA_MODINIT_FUNC extern "C" VIA_EXPORT const ModuleDef*

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

#include <fmt/core.h>
#include <fmt/format.h>
#include <mimalloc.h>
#include <cassert>
#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace via {

using std::literals::operator""s;
using std::literals::operator""sv;

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
using uptr = uintptr_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using ptr = intptr_t;

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
