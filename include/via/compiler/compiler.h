// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |
#pragma once

#include "common.h"
#include "bytecode.h"
#include "instruction.h"
#include "ast.h"
#include "visitor.h"

// ===========================================================================================
// compiler.h
// This file declares the Compiler class.
//
// The Compiler class serves as an abstract compilation interface,
//  taking away the complexity of node visitation, optimization,
//  global tracking, stack tracking, etc.
//
// The `generate` method is the main entry point for performing compilation,
//  it returns a boolean indicating if the program failed or not, of which
//  a value of `true` represents failure. The method could theoretically be called
//  multiple times, but it is not recommended to do so.
VIA_NAMESPACE_BEGIN

class Compiler final {
public:
  Compiler(TransUnitContext& unit_ctx)
      : unit_ctx(unit_ctx) {}

  // Compiler entry point.
  bool generate();

private:
  bool check_global_collisions();

private:
  TransUnitContext& unit_ctx;
};

VIA_NAMESPACE_END
