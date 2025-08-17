// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "diagnostics.h"

namespace via {

void Diagnostics::clear() {
  diags.clear();
}

void Diagnostics::emit() {
  for (const Diagnosis& diag : diags) {
    Location loc = diag.loc.to_relative(file);

    String addr = fmt::format("{}:{}:{}", path, loc.line, loc.offset);
    String msg = fmt::format(
        "{} {} [{}]", diag.msg,
        apply_color("at", FGColor::White, BGColor::Black, Style::Faint),
        apply_color(addr, FGColor::Cyan, BGColor::Black, Style::Bold));

    switch (diag.kind) {
      case Diagnosis::Kind::Info:
        spdlog::info(msg);
        break;
      case Diagnosis::Kind::Warn:
        spdlog::warn(msg);
        break;
      case Diagnosis::Kind::Error:
        spdlog::error(msg);
        break;
      default:
        break;
    }
  }
}

}  // namespace via
