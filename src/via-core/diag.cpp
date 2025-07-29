// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "diag.h"

namespace via {

namespace core {

void DiagnosticManager::clear() {
  diags.clear();
}

void DiagnosticManager::emit() {
  for (const Diagnosis& diag : diags) {
    Location loc = diag.loc.to_relative(file);
    String addr = fmt::format("{}:{}:{}", path, loc.line, loc.offset);
    String msg =
      fmt::format("{} {}", diag.msg, apply_color(addr, FGColor::Cyan, BGColor::Black, Style::Bold));

    switch (diag.kind) {
    case Diag::Info:
      spdlog::info(msg);
      break;
    case Diag::Warn:
      spdlog::warn(msg);
      break;
    case Diag::Error:
      spdlog::error(msg);
      break;
    default:
      break;
    }
  }
}

} // namespace core

} // namespace via
