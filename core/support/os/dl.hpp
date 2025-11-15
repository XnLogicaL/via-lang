/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <expected>
#include <filesystem>
#include <type_traits>
#include <via/config.hpp>
#include "support/utility.hpp"

// We assume that the <dlfcn.h> header is the proper POSIX dynamic library API
// Everything should be fine as long as the implementation is not cooked
#define HAS_DLFCN __has_include(<dlfcn.h>)

namespace via {
namespace os {

#ifdef HAS_DLFCN
VIA_CONSTANT const char* DL_EXTENSION = ".so";
#elif defined(VIA_PLATFORM_WINDOWS)
VIA_CONSTANT const char* DL_EXTENSION = ".dll";
#else
VIA_CONSTANT const char* DL_EXTENSION = "";
#endif

class DynamicLibrary final
{
  public:
    DynamicLibrary() = default;
    ~DynamicLibrary();

    IMPL_MOVE(DynamicLibrary);
    NO_COPY(DynamicLibrary);

  public:
    static std::expected<DynamicLibrary, std::string>
    load_library(std::filesystem::path path);

  public:
    std::expected<void*, std::string> load_symbol_raw(const char* symbol);

    template <typename T>
        requires std::is_pointer_v<T>
    std::expected<T, std::string> load_symbol(const char* symbol)
    {
        auto result = load_symbol_raw(symbol);
        if (result.has_value())
            return reinterpret_cast<T>(*result);
        return std::unexpected(result.error());
    }

  private:
    explicit DynamicLibrary(void* handle)
        : m_handle(handle)
    {}

  private:
    void* m_handle = nullptr;
};

} // namespace os
} // namespace via
