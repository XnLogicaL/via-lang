/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "diagnostics.h"
#include "ansi.h"
#include "magic_enum/magic_enum.hpp"

void via::DiagContext::emit(spdlog::logger* logger) const
{
  for (const auto& diag : mDiags) {
    emitOnce(diag, logger);
  }
}

void via::DiagContext::emitOnce(const Diagnosis& diag,
                                spdlog::logger* logger) const
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

  if (diag.location.begin >= mSource.size()) {
    logger->log(level, "{}", diag.message);
    return;
  }

  const char* begin = reinterpret_cast<const char*>(mSource.data());
  const char* end = begin + mSource.size();
  const char* ptr = begin + diag.location.begin;

  // Find line boundaries
  const char* lineBegin = ptr;
  while (lineBegin > begin && lineBegin[-1] != '\n' && lineBegin[-1] != '\r')
    --lineBegin;
  const char* lineEnd = ptr;
  while (lineEnd < end && *lineEnd != '\n' && *lineEnd != '\r')
    ++lineEnd;

  u64 line = 1 + std::count(begin, lineBegin, '\n');
  u64 col = static_cast<u64>(ptr - lineBegin) + 1;

  std::string_view lineView(lineBegin,
                            static_cast<size_t>(lineEnd - lineBegin));

  logger->log(level, "{} {} {}", diag.message,
              ansi::format("at", ansi::Foreground::White,
                           ansi::Background::Black, ansi::Style::Faint),
              ansi::format(std::format("[{}:{}:{}]", mPath, line, col),
                           ansi::Foreground::Cyan));

  size_t spanBegin =
    std::min(static_cast<size_t>(diag.location.begin - (lineBegin - begin)),
             lineView.size());
  size_t spanEnd =
    std::min(static_cast<size_t>(diag.location.end - (lineBegin - begin)),
             lineView.size());

  std::string highlightedLine;
  if (spanBegin < spanEnd) {
    highlightedLine.reserve(lineView.size() + 32);
    highlightedLine.append(lineView.substr(0, spanBegin));
    highlightedLine.append(
      ansi::format(std::string(lineView.substr(spanBegin, spanEnd - spanBegin)),
                   foreground, ansi::Background::Black, ansi::Style::Bold));
    highlightedLine.append(lineView.substr(spanEnd));
  } else {
    highlightedLine = std::string(lineView);
  }

  spdlog::set_pattern("%v");
  size_t lineWidth = static_cast<size_t>(std::log10(line)) + 1;
  logger->log(spdlog::level::off, " {} | {}", line, highlightedLine);

  std::string caret(lineView.size(), ' ');
  if (spanBegin < spanEnd) {
    std::fill(caret.begin() + spanBegin, caret.begin() + spanEnd, '^');
  } else if (col > 0 && col - 1 < caret.size()) {
    caret[col - 1] = '^';
  }

  // Trim trailing spaces in caret
  caret.erase(std::find_if(caret.rbegin(), caret.rend(),
                           [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
              caret.end());

  logger->log(
    spdlog::level::off,
    std::format(" {0} | {1}{2}\n {0} |", std::string(lineWidth, ' '),
                ansi::format(caret, foreground, ansi::Background::Black,
                             ansi::Style::Bold),
                diag.footnote.valid
                  ? std::format("-- {} {}", toString(diag.footnote.kind),
                                diag.footnote.message)
                  : ""));

  spdlog::set_pattern("%^%l:%$ %v");
}
