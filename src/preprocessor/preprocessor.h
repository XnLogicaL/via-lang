// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_PREPROC_H
#define VIA_HAS_HEADER_PREPROC_H

#include "error-bus.h"
#include "stack.h"
#include "constant.h"
#include "bytecode.h"
#include "ast.h"
#include "common.h"
#include "token.h"

namespace via {

class Preprocessor final {
public:
  ~Preprocessor() = default;
  Preprocessor(TransUnitContext& unit_ctx)
    : unit_ctx(unit_ctx) {}

  bool preprocess();
  void declare_default();

private:
  [[maybe_unused]] CErrorBus err_bus;
  [[maybe_unused]] TransUnitContext& unit_ctx;
};

} // namespace via

#endif
