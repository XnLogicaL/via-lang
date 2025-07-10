// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_DIAG_H
#define VIA_DIAG_H

#include "common.h"
#include "lextoken.h"
#include <spdlog/spdlog.h>

namespace via {

template<typename... Args>
using Fmt = std::format_string<Args...>;

enum DiagnosisKind {
  DK_INFO,
  DK_WARN,
  DK_ERROR,
};

struct Diagnosis {
  DiagnosisKind kind;
  Location loc;
  String msg;
};

struct DiagContext {
  Vec<Diagnosis> diags;
};

void diag_raw(DiagContext& ctx, Diagnosis&& diagnosis);
void diag_emit(const DiagContext& ctx);
void diag_clear(DiagContext& ctx);

template<const DiagnosisKind Kind>
void diag(DiagContext& ctx, Location loc, String msg) {
  diag_raw(ctx, {Kind, loc, msg});
}

template<const DiagnosisKind Kind, typename... Args>
void diagf(DiagContext& ctx, Location loc, Fmt<Args...> fmt, Args... args) {
  diag_raw(ctx, {Kind, loc, std::vformat(fmt, std::forward<Args>(args)...)});
}

template<typename T = std::function<bool(const Diagnosis&)>>
Vec<const Diagnosis*> diag_filter(const DiagContext& ctx, T callback) {
  Vec<const Diagnosis*> filtered;

  for (const Diagnosis& diag : ctx.diags)
    if (callback(diag))
      filtered.push_back(&diag);

  return filtered;
}

} // namespace via

#endif
