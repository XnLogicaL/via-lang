//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#include "compiler.h"
#include "compiler-types.h"
#include "state.h"
#include "string-utility.h"

// ===========================================================================================
// compiler.cpp
//
namespace via {

void Compiler::codegen_prep() {
  unit_ctx.internal.globals->declare_builtins();
}

void Compiler::insert_exit0_instruction() {
  unit_ctx.bytecode->emit(IOpCode::EXIT, {0});
}

bool Compiler::generate() {
  CErrorBus emitter;
  RegisterAllocator allocator(VIA_ALL_REGISTERS, true);
  StmtNodeVisitor visitor(unit_ctx, emitter, allocator);

  bool failed = false;

  codegen_prep();

  for (StmtNodeBase*& stmt : unit_ctx.ast->statements) {
    if (DeferStmtNode* defer_stmt = get_derived_instance<StmtNodeBase, DeferStmtNode>(stmt)) {
      visitor.compiler_error(
        defer_stmt->begin, defer_stmt->end, "Defer statements are not allowed within global scope"
      );
      visitor.compiler_output_end();
      continue;
    }

    stmt->accept(visitor);
  }

  insert_exit0_instruction();

  size_t errors_generated = visitor.get_compiler_error_count();
  if (errors_generated > 0) {
    visitor.compiler_error(std::format("{} error(s) generated.", errors_generated));
  }

  return failed || visitor.failed();
}

} // namespace via
