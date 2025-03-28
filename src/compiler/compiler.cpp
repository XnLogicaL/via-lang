// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "compiler.h"
#include "state.h"
#include "string-utility.h"

// ===========================================================================================
// compiler.cpp
//
VIA_NAMESPACE_BEGIN

bool Compiler::generate() {
  RegisterAllocator allocator(VIA_REGISTER_COUNT, true);
  ErrorBus emitter;
  StmtVisitor visitor(unit_ctx, emitter, allocator);

  unit_ctx.internal.globals->declare_builtins();

  for (pStmtNode& stmt : unit_ctx.ast->statements) {
    stmt->accept(visitor);
  }

  if (check_global_collisions()) {
    return true;
  }

  unit_ctx.internal.label_count = visitor.label_counter;

  return visitor.failed();
}

bool Compiler::check_global_collisions() {
  return false;
}

VIA_NAMESPACE_END
