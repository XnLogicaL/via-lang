/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "diagnostics.h"
#include "color.h"

namespace via
{

void DiagContext::emit(spdlog::logger* logger) const
{
  for (const auto& d : mDiags) {
    emitOnce(d, logger);
  }
}

void DiagContext::emitOnce(const Diagnosis& d, spdlog::logger* logger) const
{
  spdlog::level::level_enum level;
  switch (d.kind) {
    case Diagnosis::Kind::Info:
      level = spdlog::level::info;
      break;
    case Diagnosis::Kind::Warn:
      level = spdlog::level::warn;
      break;
    case Diagnosis::Kind::Error:
      level = spdlog::level::err;
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
  while (lineBegin > begin && lineBegin[-1] != '\n' && lineBegin[-1] != '\r') {
    --lineBegin;
  }

  const char* lineEnd = ptr;
  while (lineEnd < end && *lineEnd != '\n' && *lineEnd != '\r') {
    ++lineEnd;
  }

  for (const char* p = begin; p < lineBegin; ++p) {
    if (*p == '\n')
      ++line;
  }

  ++line;                                       // 1-based
  col = static_cast<u64>(ptr - lineBegin) + 1;  // 1-based
  lineView =
    std::string_view(lineBegin, static_cast<usize>(lineEnd - lineBegin));

  logger->log(
    level, "{} {} {}", d.msg,
    ansiFormat("at", Fg::White, Bg::Black, Style::Faint),
    ansiFormat(fmt::format("[{}:{}:{}] module({})", mPath, line, col, mName),
               Fg::Cyan));

  usize lineWidth = static_cast<usize>(std::log10(line)) + 1;

  spdlog::set_pattern("%v");
  logger->log(spdlog::level::off, " {} | {}", line, lineView);

  std::string caret(lineView.size(), ' ');
  if (col > 0 && col - 1 < caret.size()) {
    caret[col - 1] = '^';
  }

  logger->log(spdlog::level::off, " {} | {}", std::string(lineWidth, ' '),
              caret);
  spdlog::set_pattern("%^%l:%$ %v");
}

}  // namespace via
