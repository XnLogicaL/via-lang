/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "dl.hpp"
#include <expected>
#include <filesystem>
#include <string>

#ifdef HAS_DLFCN
    #include <dlfcn.h>
    #define USE_DLFCN 1
#else
    #ifdef VIA_PLATFORM_WINDOWS
        #include <windows.h>
    #endif
    #define USE_DLFCN 0
#endif

#if !USE_DLFCN && defined(VIA_PLATFORM_WINDOWS)
static std::string dlerror_win()
{
    DWORD error = GetLastError();
    if (!error) {
        return {};
    }

    char* msg = nullptr;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&msg),
        0,
        nullptr
    );

    std::string result = msg ? msg : "Unknown error";
    if (msg) {
        LocalFree(msg);
    }
    return result;
}
#endif

std::expected<via::os::DynamicLibrary, std::string>
via::os::DynamicLibrary::load_library(std::filesystem::path path)
{
    std::string path_str = path.string();

    if (!std::filesystem::is_regular_file(path)) {
        return std::unexpected(std::format("No such file or directory: '{}'", path_str));
    }

    if (path.extension().string() != DL_EXTENSION) {
        return std::unexpected(
            std::format(
                "Dynamic library has invalid extension (expected {})",
                DL_EXTENSION
            )
        );
    }

#if USE_DLFCN
    if (void* handle = dlopen(path_str.c_str(), RTLD_NOW))
        return DynamicLibrary(handle);
    const char* err = dlerror();
    return std::unexpected(err ? err : "Unknown dlopen error");
#else
    if (HMODULE handle = LoadLibraryA(path_str.c_str()))
        return DynamicLibrary(reinterpret_cast<void*>(handle));
    return std::unexpected(dlerror_win());
#endif
}

via::os::DynamicLibrary::~DynamicLibrary()
{
    if (m_handle != nullptr) {
#if USE_DLFCN
        dlclose(m_handle);
#else
        FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
#endif
        m_handle = nullptr;
    }
}

via::os::DynamicLibrary::DynamicLibrary(DynamicLibrary&& other)
    : m_handle(other.m_handle)
{
    other.m_handle = nullptr;
}

via::os::DynamicLibrary& via::os::DynamicLibrary::operator=(DynamicLibrary&& other)
{
    if (this != &other) {
        if (m_handle) {
#if USE_DLFCN
            dlclose(m_handle);
#else
            FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
#endif
        }
        m_handle = other.m_handle;
        other.m_handle = nullptr;
    }
    return *this;
}

std::expected<void*, std::string>
via::os::DynamicLibrary::load_symbol_raw(const char* symbol)
{
#if USE_DLFCN
    dlerror(); // Clear previous errors
    if (void* handle = dlsym(m_handle, symbol)) {
        return handle;
    }
    const char* err = dlerror();
    return std::unexpected(err ? err : "Unknown dlsym error");
#else
    if (FARPROC sym = GetProcAddress(reinterpret_cast<HMODULE>(m_handle), symbol)) {
        return reinterpret_cast<void*>(sym);
    }
    return std::unexpected(dlerror_win());
#endif
}
