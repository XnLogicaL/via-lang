/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "diagnostics.h"
#include "support/ansi.h"

void via::DiagContext::emit(spdlog::logger* logger) const
{
    for (const auto& diag: m_diags) {
        emitOnce(diag, logger);
    }
}

void via::DiagContext::emitOnce(const Diagnosis& diag, spdlog::logger* logger) const
{
    ansi::Foreground foreground;
    spdlog::level::level_enum level;

    switch (diag.level) {
        case Level::INFO:
            level = spdlog::level::info;
            foreground = ansi::Foreground::Cyan;
            break;
        case Level::WARNING:
            level = spdlog::level::warn;
            foreground = ansi::Foreground::Yellow;
            break;
        case Level::ERROR:
            level = spdlog::level::err;
            foreground = ansi::Foreground::Red;
            break;
    }

    if (diag.location.begin >= m_source.size()) {
        logger->log(level, "{}", diag.message);
        return;
    }

    const char* begin = reinterpret_cast<const char*>(m_source.data());
    const char* end = begin + m_source.size();
    const char* ptr = begin + diag.location.begin;

    // Find line boundaries
    const char* line_begin = ptr;
    while (line_begin > begin && line_begin[-1] != '\n' && line_begin[-1] != '\r')
        --line_begin;
    const char* line_end = ptr;
    while (line_end < end && *line_end != '\n' && *line_end != '\r')
        ++line_end;

    u64 line = 1 + std::count(begin, line_begin, '\n');
    u64 col = static_cast<u64>(ptr - line_begin) + 1;

    std::string_view line_view(line_begin, static_cast<usize>(line_end - line_begin));

    logger->log(
        level,
        "{} {} {}",
        diag.message,
        ansi::format("at", ansi::Foreground::White, ansi::Background::Black, ansi::Style::Faint),
        ansi::format(std::format("[{}:{}:{}]", m_path, line, col), ansi::Foreground::Cyan)
    );

    usize span_begin = std::min(static_cast<usize>(diag.location.begin - (line_begin - begin)), line_view.size());
    usize span_end = std::min(static_cast<usize>(diag.location.end - (line_begin - begin)), line_view.size());

    std::string hl_line;
    if (span_begin < span_end) {
        hl_line.reserve(line_view.size() + 32);
        hl_line.append(line_view.substr(0, span_begin));
        hl_line.append(ansi::format(
            std::string(line_view.substr(span_begin, span_end - span_begin)),
            foreground,
            ansi::Background::Black,
            ansi::Style::Bold
        ));
        hl_line.append(line_view.substr(span_end));
    }
    else {
        hl_line = std::string(line_view);
    }

    spdlog::set_pattern("%v");
    usize line_width = static_cast<usize>(std::log10(line)) + 1;
    logger->log(spdlog::level::off, " {} | {}", line, hl_line);

    std::string caret(line_view.size(), ' ');
    if (span_begin < span_end) {
        std::fill(caret.begin() + span_begin, caret.begin() + span_end, '^');
    }
    else if (col > 0 && col - 1 < caret.size()) {
        caret[col - 1] = '^';
    }

    // Trim trailing spaces in caret
    caret.erase(
        std::find_if(caret.rbegin(), caret.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
        caret.end()
    );

    logger->log(
        spdlog::level::off,
        std::format(
            " {0} | {1}{2}\n {0} |",
            std::string(line_width, ' '),
            ansi::format(caret, foreground, ansi::Background::Black, ansi::Style::Bold),
            diag.footnote.valid ? std::format("-- {} {}", to_string(diag.footnote.kind), diag.footnote.message) : ""
        )
    );

    spdlog::set_pattern("%^%l:%$ %v");
}
