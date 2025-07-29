// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "diag.h"

namespace via {

namespace core {

void diag_raw(DiagContext& ctx, Diagnosis&& diagnosis) {
  ctx.diags.push_back(std::move(diagnosis));
}

void diag_emit(const DiagContext& ctx) {
  for (const Diagnosis& diag : ctx.diags) {
    Location loc = diag.loc.to_relative(ctx.file);
    String addr = fmt::format("{}:{}:{}", ctx.path, loc.line, loc.offset);
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

void diag_clear(DiagContext& ctx) {
  ctx.diags.clear();
}

} // namespace core

} // namespace via
