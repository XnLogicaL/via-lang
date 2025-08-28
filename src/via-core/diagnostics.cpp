// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "diagnostics.h"
#include "color.h"

namespace via
{

void DiagnosticContext::emit(spdlog::logger* logger) const
{
  for (const auto& d : m_diags) {
    emit_one(d, logger);
  }
}

void DiagnosticContext::emit_one(const Diagnosis& d,
                                 spdlog::logger* logger) const
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
  StringView line_sv;

  if (d.loc.begin >= m_source.size()) {
    logger->log(level, "{}", d.msg);
    return;
  }

  const char* begin = reinterpret_cast<const char*>(m_source.data());
  const char* end = begin + m_source.size();
  const char* ptr = begin + d.loc.begin;

  const char* line_start = ptr;
  while (line_start > begin && line_start[-1] != '\n' &&
         line_start[-1] != '\r') {
    --line_start;
  }

  const char* line_end = ptr;
  while (line_end < end && *line_end != '\n' && *line_end != '\r') {
    ++line_end;
  }

  for (const char* p = begin; p < line_start; ++p) {
    if (*p == '\n')
      ++line;
  }

  ++line;                                        // 1-based
  col = static_cast<u64>(ptr - line_start) + 1;  // 1-based
  line_sv = StringView(line_start, static_cast<usize>(line_end - line_start));

  logger->log(
      level, "{} {} {}", d.msg,
      apply_ansi_style("at", Fg::White, Bg::Black, Style::Faint),
      apply_ansi_style(fmt::format("[{}:{}:{}]", m_path, line, col), Fg::Cyan));

  logger->log(spdlog::level::off, "{}", line_sv);

  String caret(line_sv.size(), ' ');
  if (col > 0 && col - 1 < caret.size()) {
    caret[col - 1] = '^';
  }

  logger->log(spdlog::level::off, "{}", caret);
}

}  // namespace via
