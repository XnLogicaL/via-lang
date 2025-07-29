// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_DIAG_H_
#define VIA_CORE_DIAG_H_

#include <via/config.h>
#include <via/util/color.h>
#include "lexer/location.h"
#include <spdlog/spdlog.h>
#include <fmt/core.h>

namespace via {

namespace core {

using lex::AbsLocation;
using lex::Location;

enum DiagnosisKind {
  DK_INFO,
  DK_WARN,
  DK_ERROR,
};

struct Diagnosis {
  DiagnosisKind kind;
  AbsLocation loc;
  String msg;
};

struct DiagContext {
  const String& path;
  const FileBuf& file;
  Vec<Diagnosis> diags;
};

void diag_raw(DiagContext& ctx, Diagnosis&& diagnosis);
void diag_emit(const DiagContext& ctx);
void diag_clear(DiagContext& ctx);

template<const DiagnosisKind Kind>
void diag(DiagContext& ctx, AbsLocation loc, String msg) {
  diag_raw(ctx, {Kind, loc, msg});
}

template<const DiagnosisKind Kind, typename... Args>
void diagf(DiagContext& ctx, AbsLocation loc, Fmt<Args...> fmt, Args... args) {
  diag_raw(ctx, {Kind, loc, fmt::format(fmt, std::forward<Args>(args)...)});
}

template<typename T = std::function<bool(const Diagnosis&)>>
Vec<const Diagnosis*> diag_filter(const DiagContext& ctx, T callback) {
  Vec<const Diagnosis*> filtered;

  for (const Diagnosis& diag : ctx.diags)
    if (callback(diag))
      filtered.push_back(&diag);

  return filtered;
}

} // namespace core

} // namespace via

#endif
