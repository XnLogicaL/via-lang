// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "diagnostics.h"
#include "color.h"

namespace via {

inline void Diagnostics::emit(spdlog::logger* logger) const {
  for (const auto& d : m_diags)
    emit_one(d, logger);
}

void Diagnostics::emit_one(const Diagnosis& d, spdlog::logger* logger) const {
  const char* tag = NULL;
  switch (d.kind) {
    case Diagnosis::Kind::Info:
      tag = "info";
      break;
    case Diagnosis::Kind::Warn:
      tag = "warn";
      break;
    case Diagnosis::Kind::Error:
      tag = "error";
      break;
  }

  const bool loc_ok = d.loc.begin < m_file.size();

  u64 line = 0, col = 0;
  StringView line_sv;
  if (loc_ok) {
    const char* begin = reinterpret_cast<const char*>(m_file.data());
    const char* end = begin + m_file.size();
    const char* ptr = begin + d.loc.begin;

    const char* line_start = ptr;
    while (line_start > begin && line_start[-1] != '\n' &&
           line_start[-1] != '\r')
      --line_start;

    const char* line_end = ptr;
    while (line_end < end && *line_end != '\n' && *line_end != '\r')
      ++line_end;

    for (const char* p = begin; p < line_start; ++p)
      if (*p == '\n')
        ++line;
    ++line;  // 1-based

    col = static_cast<u64>(ptr - line_start) + 1;  // 1-based
    line_sv =
        StringView(line_start, static_cast<size_t>(line_end - line_start));
  }

  if (!loc_ok) {
    logger->log(spdlog::level::info, "{}: {}", tag, d.msg);
    return;
  }

  logger->log(spdlog::level::info, "{}:{}:{}: {}: {}", m_path, line, col, tag,
              d.msg);
  logger->log(spdlog::level::info, "{}", line_sv);

  String caret(line_sv.size(), ' ');

  if (col > 0 && col - 1 < caret.size())
    caret[col - 1] = '^';

  logger->log(spdlog::level::info, "{}", caret);
}

}  // namespace via
