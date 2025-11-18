/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "ansi.hpp"
#include <cstring>

#ifdef VIA_PLATFORM_UNIX
    #include <unistd.h>

bool via::ansi::detail::is_ansi_supported() noexcept
{
    if (isatty(fileno(stdout)))
        return true;
    if (const char* term = std::getenv("TERM"))
        return strcmp(term, "dumb") != 0;
    return false;
}
#elifdef VIA_PLATFORM_WINDOWS
    #include <windows.h>

bool via::ansi::detail::is_ansi_supported() noexcept
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        return false;
    }

    // enable virtual terminal processing
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        return false;
    }

    return true;
}
#else
bool via::ansi::detail::is_ansi_supported() noexcept
{
    Logger::stdout_logger().warn(
        "host terminal does not support ANSI escape codes, compiler output may be "
        "unreadable"
    );
    return false;
}
#endif

std::string
via::ansi::format(std::string string, Foreground fg, Background bg, Style style)
{
    static bool supported = detail::is_ansi_supported();
    if (!supported)
        return string;

    std::string codes;

    if (style != Style::NONE)
        codes += std::to_string(static_cast<int>(style));

    if (fg != Foreground::NONE) {
        if (!codes.empty())
            codes += ";";
        codes += std::to_string(static_cast<int>(fg));
    }

    if (bg != Background::NONE) {
        if (!codes.empty())
            codes += ";";
        codes += std::to_string(static_cast<int>(bg));
    }

    if (!codes.empty())
        return "\033[" + codes + "m" + string + "\033[0m";
    return string;
}

std::string via::ansi::bold(std::string string)
{
    return format(string, Foreground::NONE, Background::NONE, Style::BOLD);
}

std::string via::ansi::italic(std::string string)
{
    return format(string, Foreground::NONE, Background::NONE, Style::ITALIC);
}

std::string via::ansi::faint(std::string string)
{
    return format(string, Foreground::NONE, Background::NONE, Style::FAINT);
}
