// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "diag.h"

namespace via {

void diag_raw(DiagContext& ctx, Diagnosis&& diagnosis) {
  ctx.diags.push_back(std::move(diagnosis));
}

void diag_emit(const DiagContext& ctx) {
  for (const Diagnosis& diagnosis : ctx.diags) {
    Location loc = abs_location_translate(ctx.file, diagnosis.loc.begin);
    String sloc = std::format(" {}:{}:{}", diagnosis.file, loc.line, loc.offset);

    switch (diagnosis.kind) {
    case DK_INFO:
      spdlog::info(diagnosis.msg + sloc);
      break;
    case DK_WARN:
      spdlog::warn(diagnosis.msg + sloc);
      break;
    case DK_ERROR:
      spdlog::error(diagnosis.msg + sloc);
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
