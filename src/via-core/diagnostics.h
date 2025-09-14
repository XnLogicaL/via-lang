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
#include "ansi.h"
#include "lexer/location.h"
#include "utility.h"

namespace via
{

enum class Level : u8
{
  INFO,
  WARNING,
  ERROR,
};

struct Footnote
{
  const enum class Kind : u8 {
    NOTE,
    HINT,
  } kind = Kind::NOTE;

  const bool valid = false;
  const std::string message = "";

  Footnote() = default;
  Footnote(Kind kind, std::string message)
      : kind(kind), valid(true), message(message)
  {}
};

struct Diagnosis
{
  const Level level;
  const SourceLoc location;   // Absolute location in the source buffer
  const std::string message;  // Human-readable message
  const Footnote footnote = {};
};

class DiagContext final
{
 public:
  DiagContext(std::string path, std::string name, const std::string& source)
      : mPath(path), mName(name), mSource(source)
  {}

  NO_COPY(DiagContext)

 public:
  void emit(spdlog::logger* logger = spdlog::default_logger().get()) const;
  void clear() noexcept { mDiags.clear(); }

  void report(Diagnosis diag) noexcept { mDiags.push_back(std::move(diag)); }

  template <Level Lv>
  void report(SourceLoc location, std::string message, Footnote footnote = {})
  {
    mDiags.emplace_back(Lv, location, message, footnote);
  }

  [[nodiscard]] auto& diagnostics() noexcept { return mDiags; }
  [[nodiscard]] const auto& diagnostics() const noexcept { return mDiags; }

  [[nodiscard]] bool hasErrors() const noexcept
  {
    for (const auto& diag : mDiags) {
      if (diag.level == Level::ERROR)
        return true;
    }

    return false;
  }

  [[nodiscard]] const std::string& source() const noexcept { return mSource; }

 private:
  // Helper to pretty-print a single diagnosis line with source context.
  void emitOnce(const Diagnosis& d, spdlog::logger* logger) const;

 private:
  std::string mPath, mName;
  const std::string& mSource;
  std::vector<Diagnosis> mDiags{};
};

inline std::string toString(Level level) noexcept
{
  switch (level) {
    case Level::INFO:
      return ansi::format("info:", ansi::Foreground::Cyan,
                          ansi::Background::Black, ansi::Style::Bold);
    case Level::WARNING:
      return ansi::format("warning:", ansi::Foreground::Yellow,
                          ansi::Background::Black, ansi::Style::Bold);
    case Level::ERROR:
      return ansi::format("error:", ansi::Foreground::Red,
                          ansi::Background::Black, ansi::Style::Bold);
  }
}

inline std::string toString(Footnote::Kind kind) noexcept
{
  switch (kind) {
    case Footnote::Kind::HINT:
      return ansi::format("hint:", ansi::Foreground::Green,
                          ansi::Background::Black, ansi::Style::Bold);
    case Footnote::Kind::NOTE:
      return ansi::format("note:", ansi::Foreground::Blue,
                          ansi::Background::Black, ansi::Style::Bold);
  }
}

}  // namespace via
