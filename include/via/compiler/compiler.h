//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_COMPILER_H
#define VIA_HAS_HEADER_COMPILER_H

#include "common.h"
#include "bytecode.h"
#include "instruction.h"
#include "ast.h"
#include "visitor.h"

// ===========================================================================================
// compiler.h
// This file declares the compiler class.
//
// The compiler class serves as an abstract compilation interface,
//  taking away the complexity of node visitation, optimization,
//  global tracking, stack tracking, etc.
//
// The `generate` method is the main entry point for performing compilation,
//  it returns a boolean indicating if the program failed or not, of which
//  a value of `true` represents failure. The method could theoretically be called
//  multiple times, but it is not recommended to do so.
namespace via {

class Compiler final {
public:
  Compiler(TransUnitContext& unit_ctx)
    : unit_ctx(unit_ctx) {}

  // Compiler entry point.
  bool generate();

  // Adds a custom unused expression handler to the statement visitor.
  void add_unused_expression_handler(const unused_expression_handler_t& handler);

private:
  void codegen_prep();
  void insert_exit0_instruction();

private:
  TransUnitContext& unit_ctx;
};

} // namespace via

#endif
