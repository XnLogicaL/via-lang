/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "logger.hpp"
#include "debug.hpp"
#include "support/ansi.hpp"

static std::optional<std::string> get_level_string(via::LogLevel level)
{
    using enum via::LogLevel;
    using namespace via::ansi;

    switch (level) {
    case INFO:
        return format("info:", Foreground::BLUE, Background::NONE, Style::BOLD);
    case WARN:
        return format("warning:", Foreground::YELLOW, Background::NONE, Style::BOLD);
    case ERROR:
        return format("error:", Foreground::RED, Background::NONE, Style::BOLD);
    default:
        break;
    }

    return std::nullopt;
}

void via::Logger::log(via::LogLevel level, std::string string)
{
    if (auto header = get_level_string(level)) {
        m_file << *header << " " << string;
    } else {
        m_file << string;
    }
}
