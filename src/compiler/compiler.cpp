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

void Compiler::codegen_prep() {
  unit_ctx.internal.globals->declare_builtins();
}

void Compiler::check_global_collisions(bool& failed) {
  failed = false;
}

void Compiler::insert_exit0_instruction() {
  unit_ctx.bytecode->emit(OpCode::EXIT, {0});
}

bool Compiler::generate() {
  ErrorBus emitter;
  RegisterAllocator allocator(VIA_REGISTER_COUNT, true);
  StmtVisitor visitor(unit_ctx, emitter, allocator);

  bool failed = false;

  codegen_prep();

  for (pStmtNode& stmt : unit_ctx.ast->statements) {
    stmt->accept(visitor);
  }

  insert_exit0_instruction();
  check_global_collisions(failed);

  return failed || visitor.failed();
}

VIA_NAMESPACE_END
