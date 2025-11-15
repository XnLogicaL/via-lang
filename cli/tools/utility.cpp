/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "utility.hpp"

// Expand $HOME on Unix, %USERPROFILE% on Windows.
std::filesystem::path via::get_home_dir()
{
#ifdef _WIN32
    if (const char* profile = std::getenv("USERPROFILE")) {
        return std::filesystem::path(profile);
    }
    // Fallback: create from HOMEDRIVE + HOMEPATH
    const char* drive = std::getenv("HOMEDRIVE");
    const char* path = std::getenv("HOMEPATH");
    if (drive && path) {
        return std::filesystem::path(std::string(drive) + path);
    }
    return std::filesystem::current_path(); // last resort
#else
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home);
    }
    return std::filesystem::current_path(); // last resort
#endif
}

// Gets the base directory where via stores core stuff
std::filesystem::path via::get_lang_dir()
{
#ifdef _WIN32
    if (const char* local = std::getenv("LOCALAPPDATA")) {
        std::filesystem::path user_dir = std::filesystem::path(local) / "via";
        if (std::filesystem::exists(user_dir))
            return user_dir;
    }
    std::filesystem::path fallback = get_home_dir() / "AppData" / "Local" / "via";
    if (std::filesystem::exists(fallback))
        return fallback;
    return fallback;

#else
    if (const char* xdg = std::getenv("XDG_DATA_HOME")) {
        std::filesystem::path user_dir = std::filesystem::path(xdg) / "via";
        if (std::filesystem::exists(user_dir))
            return user_dir;
    }
    std::filesystem::path user_dir = get_home_dir() / ".local" / "share" / "via";
    if (std::filesystem::exists(user_dir))
        return user_dir;
    std::filesystem::path sys_dir = "/usr/local/share/via";
    if (std::filesystem::exists(sys_dir))
        return sys_dir;
    sys_dir = "/usr/share/via";
    return sys_dir;
#endif
}
