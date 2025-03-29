// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _vl_compiler_h
#define _vl_compiler_h

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

class compiler final {
public:
  compiler(trans_unit_context& unit_ctx)
    : unit_ctx(unit_ctx) {}

  // compiler entry point.
  bool generate();

private:
  void codegen_prep();
  void check_global_collisions(bool& failed);
  void insert_exit0_instruction();

private:
  trans_unit_context& unit_ctx;
};

} // namespace via

#endif
