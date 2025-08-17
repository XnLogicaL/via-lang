// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_DIAG_H_
#define VIA_CORE_DIAG_H_

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <via/config.h>
#include <via/types.h>
#include "lexer/location.h"

namespace via {

struct Diagnosis {
  enum class Kind {
    Info,
    Warn,
    Error,
  } kind;

  AbsLocation loc;
  String msg;
};

class Diagnostics final {
 public:
  Diagnostics(const String& path, const FileBuf& file)
      : path(path), file(file) {}

 public:
  void emit();
  void clear();

  template <const Diagnosis::Kind Kind>
  void diagnose(AbsLocation loc, String msg) {
    diags.push_back({Kind, loc, msg});
  }

  template <const Diagnosis::Kind Kind, typename... Args>
  void diagnosef(AbsLocation loc,
                 fmt::format_string<Args...> fmt,
                 Args... args) {
    diags.push_back({Kind, loc, fmt::format(fmt, std::forward<Args>(args)...)});
  }

  Vec<Diagnosis>& get_diagnostics() { return diags; }

 private:
  const String& path;
  const FileBuf& file;
  Vec<Diagnosis> diags;
};

}  // namespace via

#endif
