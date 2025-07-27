// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "diag.h"

namespace via {

void diag_raw(DiagContext& ctx, Diagnosis&& diagnosis) {
  ctx.diags.push_back(std::move(diagnosis));
}

void diag_emit(const DiagContext& ctx) {
  for (const Diagnosis& diag : ctx.diags) {
    Location loc = abs_location_translate(ctx.file, diag.loc.begin);
    String addr = std::format("{}:{}:{}", ctx.path, loc.line, loc.offset);
    String msg = std::format("{} {}", diag.msg, apply_color(addr, FG_CYAN, BG_BLACK, ST_BOLD));

    switch (diag.kind) {
    case DK_INFO:
      spdlog::info(msg);
      break;
    case DK_WARN:
      spdlog::warn(msg);
      break;
    case DK_ERROR:
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

} // namespace via
