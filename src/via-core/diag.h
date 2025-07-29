// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_DIAG_H_
#define VIA_CORE_DIAG_H_

#include <via/config.h>
#include <util/color.h>
#include "lexer/location.h"
#include <spdlog/spdlog.h>
#include <fmt/core.h>

namespace via {

namespace core {

using lex::AbsLocation;
using lex::Location;

enum class Diag {
  Info,
  Warn,
  Error,
};

struct Diagnosis {
  Diag kind;
  AbsLocation loc;
  String msg;
};

class DiagnosticManager {
public:
  inline explicit DiagnosticManager(const String& path, const FileBuf& file)
    : path(path),
      file(file) {}

  void emit();
  void clear();

  template<const Diag Kind>
  void diagnose(AbsLocation loc, String msg) {
    diags.push_back({Kind, loc, msg});
  }

  template<const Diag Kind, typename... Args>
  void diagnosef(AbsLocation loc, Fmt<Args...> fmt, Args... args) {
    diagnose_raw({Kind, loc, fmt::format(fmt, std::forward<Args>(args)...)});
  }

  template<typename T = std::function<bool(const Diagnosis&)>>
  Vec<Diagnosis> collect(T callback) {
    Vec<Diagnosis> filtered;

    for (Diagnosis diag : diags)
      if (callback(diag))
        filtered.push_back(diag);

    return filtered;
  }

private:
  const String& path;
  const FileBuf& file;
  Vec<Diagnosis> diags;
};

} // namespace core

} // namespace via

#endif
