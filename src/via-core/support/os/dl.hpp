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
#include <via/config.hpp>
#include "support/utility.hpp"

namespace via {
namespace os {

#ifdef VIA_PLATFORM_POSIX
    #define USE_DLFCN 1
VIA_CONSTANT const char* DL_EXTENSION = ".so";
#elif defined(VIA_PLATFORM_WINDOWS)
    #define USE_DLFCN 0
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
    std::expected<T, std::string> load_symbol(const char* symbol)
    {
        auto result = load_symbol_raw(symbol);
        if (result.has_value()) {
            return reinterpret_cast<T>(*result);
        }
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
