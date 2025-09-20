/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "support/ansi.h"
#include <cstring>

#ifdef VIA_PLATFORM_UNIX
    #include <unistd.h>

static bool checkTerminalSupport()
{
    if (!isatty(fileno(stdout))) {
        return false;
    }

    const char* term = std::getenv("TERM");
    if (term == nullptr) {
        return false;
    }

    return strcmp(term, "dumb") != 0;
}
#elifdef VIA_PLATFORM_WINDOWS
    #include <windows.h>

static bool checkTerminalSupport()
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
static bool checkTerminalSupport()
{
    spdlog::warn(
        "host terminal does not support ANSI escape codes, compiler output may be "
        "unreadable"
    );
    return false;
}
#endif

std::string via::ansi::format(
    std::string string,
    Foreground foreground,
    Background background,
    Style style
)
{
    static bool hasTerminalSupport = checkTerminalSupport();
    if (hasTerminalSupport) {
        // Construct ANSI escape code and apply the color formatting to the text
        return "\033[" + std::to_string(static_cast<int>(style)) + ";" +
               std::to_string(static_cast<int>(foreground)) + ";" +
               std::to_string(static_cast<int>(background)) + "m" + string + "\033[0m";
    }
    else {
        return string;
    }
}
