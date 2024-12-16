/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "compiler.h"

namespace via::Compilation
{

using namespace via::Parsing;

// Compiles the AST into instructions
void Compiler::compile()
{
    bytecode = generator.generate();
}

// Optimize the bytecode by invoking the PassManager back-end
void Compiler::optimize()
{
    static bool __called__ = false;
    VIA_ASSERT(!__called__, "Compiler::optimize() called twice");
    __called__ = true;
    pass_manager.apply_all(generator, *bytecode);
}

void Compiler::add_default_passes()
{
    // Bitshift optimization
    add_pass(std::make_unique<BitShiftOptimizationPass>());
}

void Compiler::add_pass(std::unique_ptr<OptimizationPass> pass)
{
    pass_manager.add_pass(std::move(pass));
}

Compiler::Instructions_t Compiler::get()
{
    // Copy the vector to make sure it doesn't get cleaned up along with the bytecode object
    Compiler::Instructions_t vec = bytecode->get();
    return vec;
}

const Compiler::Instructions_t Compiler::get() const
{
    // Copy the vector to make sure it doesn't get cleaned up along with the bytecode object
    Compiler::Instructions_t vec = bytecode->get();
    return vec;
}

} // namespace via::Compilation