//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#include "compiler.h"
#include "state.h"
#include "string-utility.h"

// ===========================================================================================
// compiler.cpp
//
namespace via {

void compiler::codegen_prep() {
  unit_ctx.internal.globals->declare_builtins();
}

void compiler::insert_exit0_instruction() {
  unit_ctx.bytecode->emit(opcode::EXIT, {0});
}

bool compiler::generate() {
  error_bus emitter;
  register_allocator allocator(VIA_ALL_REGISTERS, true);
  stmt_node_visitor visitor(unit_ctx, emitter, allocator);

  bool failed = false;

  codegen_prep();

  for (p_stmt_node_t& stmt : unit_ctx.ast->statements) {
    stmt->accept(visitor);
  }

  insert_exit0_instruction();

  return failed || visitor.failed();
}

} // namespace via
