/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <spdlog/spdlog.h>
#include <via/config.h>
#include <via/types.h>
#include "lexer/location.h"
#include "support/ansi.h"
#include "support/utility.h"

namespace via {

enum class Level : u8
{
    INFO,
    WARNING,
    ERROR,
};

enum class FootnoteKind : u8
{
    NOTE,
    HINT,
    SUGGESTION,
};

struct Footnote
{
    const FootnoteKind kind = FootnoteKind::NOTE;

    const bool valid = false;
    const std::string message = "";

    Footnote() = default;
    Footnote(FootnoteKind kind, std::string message)
        : kind(kind),
          valid(true),
          message(message)
    {}
};

struct Diagnosis
{
    const Level level;
    const SourceLoc location;  // Absolute location in the source buffer
    const std::string message; // Human-readable message
    const Footnote footnote = {};
};

class DiagContext final
{
  public:
    DiagContext(std::string path, std::string name, const std::string& source)
        : m_path(path),
          m_name(name),
          m_source(source)
    {}

    NO_COPY(DiagContext)

  public:
    void emit(spdlog::logger* logger = spdlog::default_logger().get()) const;
    void clear() noexcept { m_diags.clear(); }
    void report(Diagnosis diag) noexcept { m_diags.push_back(std::move(diag)); }

    template <Level Lv>
    void report(SourceLoc location, std::string message, Footnote footnote = {})
    {
        m_diags.emplace_back(Lv, location, message, footnote);
    }

    [[nodiscard]] auto& diagnostics() noexcept { return m_diags; }
    [[nodiscard]] const auto& diagnostics() const noexcept { return m_diags; }
    [[nodiscard]] bool has_errors() const noexcept
    {
        for (const auto& diag: m_diags) {
            if (diag.level == Level::ERROR)
                return true;
        }

        return false;
    }

    [[nodiscard]] const std::string& source() const noexcept { return m_source; }

  private:
    // Helper to pretty-print a single diagnosis line with source context.
    void emitOnce(const Diagnosis& diag, spdlog::logger* logger) const;

  private:
    std::string m_path, m_name;
    const std::string& m_source;
    std::vector<Diagnosis> m_diags{};
};

inline std::string to_string(Level level) noexcept
{
    switch (level) {
    case Level::INFO:
        return ansi::format(
            "info:",
            ansi::Foreground::CYAN,
            ansi::Background::NONE,
            ansi::Style::BOLD
        );
    case Level::WARNING:
        return ansi::format(
            "warning:",
            ansi::Foreground::YELLOW,
            ansi::Background::NONE,
            ansi::Style::BOLD
        );
    case Level::ERROR:
        return ansi::format(
            "error:",
            ansi::Foreground::RED,
            ansi::Background::NONE,
            ansi::Style::BOLD
        );
    }
}

inline std::string to_string(FootnoteKind kind) noexcept
{
    switch (kind) {
    case FootnoteKind::HINT:
        return ansi::format(
            "hint:",
            ansi::Foreground::GREEN,
            ansi::Background::NONE,
            ansi::Style::BOLD
        );
    case FootnoteKind::NOTE:
        return ansi::format(
            "note:",
            ansi::Foreground::BLUE,
            ansi::Background::NONE,
            ansi::Style::BOLD
        );
    case FootnoteKind::SUGGESTION:
        return ansi::format(
            "suggestion:",
            ansi::Foreground::MAGENTA,
            ansi::Background::NONE,
            ansi::Style::BOLD
        );
    }
}

} // namespace via
