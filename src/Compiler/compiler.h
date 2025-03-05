// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |
#pragma once

#include "common.h"
#include "bytecode.h"
#include "cleaner.h"
#include "instruction.h"
#include "ast.h"
#include "visitor.h"

// ================================================================ |
// File compiler.h: Compiler class declaration.                     |
// ================================================================ |
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
// ================================================================ |
namespace via {

class Compiler {
public:
    Compiler(ProgramData &program)
        : program(program)
    {
    }

    bool generate();

private:
    bool check_global_collisions();

private:
    ProgramData &program;
};

} // namespace via