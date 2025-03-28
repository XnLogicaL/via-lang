// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_preproc_h
#define vl_has_header_preproc_h

#include "error-bus.h"
#include "stack.h"
#include "constant.h"
#include "bytecode.h"
#include "ast.h"
#include "common.h"
#include "token.h"

namespace via {

class preprocessor final {
public:
  ~preprocessor() = default;
  preprocessor(trans_unit_context& unit_ctx)
    : unit_ctx(unit_ctx) {}

  bool preprocess();

  void declare_default();

private:
  [[maybe_unused]] trans_unit_context& unit_ctx;

  [[maybe_unused]] error_bus err_bus;
};

} // namespace via

#endif
