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
#include <via/config.hpp>
#include "source.hpp"
#include "support/utility.hpp"

namespace via {

enum class Level : uint8_t
{
    INFO,
    WARNING,
    ERROR,
};

enum class FootnoteKind : uint8_t
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
    DiagContext(std::string path, std::string name, const SourceBuffer& source)
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

    auto& diagnostics() { return m_diags; }
    bool has_errors() const
    {
        for (const auto& diag: m_diags) {
            if (diag.level == Level::ERROR)
                return true;
        }
        return false;
    }

    auto& source() const { return m_source; }

  private:
    // Helper to pretty-print a single diagnosis line with source context.
    void emit_one(const Diagnosis& diag, spdlog::logger* logger) const;

  private:
    std::string m_path, m_name;
    const SourceBuffer& m_source;
    std::vector<Diagnosis> m_diags{};
};

std::string to_string(Level level) noexcept;
std::string to_string(FootnoteKind kind) noexcept;

} // namespace via
