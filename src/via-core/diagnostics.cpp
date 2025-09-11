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

void via::DiagContext::emit(spdlog::logger* logger) const
{
  for (const auto& d : mDiags) {
    emitOnce(d, logger);
  }
}

void via::DiagContext::emitOnce(const Diagnosis& d,
                                spdlog::logger* logger) const
{
  Fg foreground;
  spdlog::level::level_enum level;

  switch (d.kind) {
    case Diagnosis::Kind::Info:
      level = spdlog::level::info;
      foreground = Fg::Cyan;
      break;
    case Diagnosis::Kind::Warn:
      level = spdlog::level::warn;
      foreground = Fg::Yellow;
      break;
    case Diagnosis::Kind::Error:
      level = spdlog::level::err;
      foreground = Fg::Red;
      break;
  }

  u64 line = 0, col = 0;
  std::string_view lineView;

  if (d.loc.begin >= mSource.size()) {
    logger->log(level, "{}", d.msg);
    return;
  }

  const char* begin = reinterpret_cast<const char*>(mSource.data());
  const char* end = begin + mSource.size();
  const char* ptr = begin + d.loc.begin;

  const char* lineBegin = ptr;
  while (lineBegin > begin && lineBegin[-1] != '\n' && lineBegin[-1] != '\r')
    --lineBegin;
  const char* lineEnd = ptr;
  while (lineEnd < end && *lineEnd != '\n' && *lineEnd != '\r')
    ++lineEnd;

  for (const char* p = begin; p < lineBegin; ++p) {
    if (*p == '\n')
      ++line;
  }
  ++line;                                       // 1-based
  col = static_cast<u64>(ptr - lineBegin) + 1;  // 1-based
  lineView =
    std::string_view(lineBegin, static_cast<usize>(lineEnd - lineBegin));

  logger->log(level, "{} {} {}", d.msg,
              ansi("at", Fg::White, Bg::Black, Style::Faint),
              ansi(std::format("[{}:{}:{}]", mPath, line, col), Fg::Cyan));

  usize lineWidth = static_cast<usize>(std::log10(line)) + 1;

  usize spanBegin = static_cast<usize>(d.loc.begin - (lineBegin - begin));
  usize spanEnd = static_cast<usize>(d.loc.end - (lineBegin - begin));

  spanBegin = std::min(spanBegin, lineView.size());
  spanEnd = std::min(spanEnd, lineView.size());

  std::string highlightedLine;
  if (spanBegin < spanEnd) {
    highlightedLine.reserve(lineView.size() + 32);
    highlightedLine.append(lineView.substr(0, spanBegin));
    highlightedLine.append(
      ansi(std::string(lineView.substr(spanBegin, spanEnd - spanBegin)),
           foreground, Bg::Black, Style::Bold));
    highlightedLine.append(lineView.substr(spanEnd));
  } else {
    highlightedLine = std::string(lineView);
  }

  spdlog::set_pattern("%v");
  logger->log(spdlog::level::off, " {} | {}", line, highlightedLine);

  std::string caret(lineView.size(), ' ');
  if (spanBegin < spanEnd) {
    for (usize i = spanBegin; i < spanEnd; ++i)
      caret[i] = '^';
  } else if (col > 0 && col - 1 < caret.size()) {
    caret[col - 1] = '^';
  }

  logger->log(spdlog::level::off, " {} | {}", std::string(lineWidth, ' '),
              ansi(caret, foreground, Bg::Black, Style::Bold));

  spdlog::set_pattern("%^%l:%$ %v");
}
