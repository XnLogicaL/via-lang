// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_PREPROC_H
#define _VIA_PREPROC_H

#include "error-bus.h"
#include "stack.h"
#include "constant.h"
#include "bytecode.h"
#include "ast.h"
#include "common.h"
#include "token.h"

VIA_NAMESPACE_BEGIN

class Preprocessor final {
public:
  ~Preprocessor() = default;
  Preprocessor(TransUnitContext& unit_ctx)
      : unit_ctx(unit_ctx) {}

  bool preprocess();

  void declare_default();

private:
  [[maybe_unused]] TransUnitContext& unit_ctx;

  [[maybe_unused]] ErrorBus err_bus;
};

VIA_NAMESPACE_END

#endif
