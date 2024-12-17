/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "compiler.h"
#include "bshift.h"
#include "constfold.h"

namespace via::Compilation
{

using namespace via::Parsing;

// Compiles the AST into instructions
void Compiler::generate()
{
    static bool __called__ = false;
    VIA_ASSERT(!__called__, "Compiler::generate() called twice");
    __called__ = true;

    // Optimize AST before generating bytecode
    pass_manager.apply_astree(generator, *bytecode);
    bytecode = generator.generate();
    pass_manager.apply_bytecode(generator, *bytecode);
}

void Compiler::add_default_passes()
{
    // Bitshift optimization
    add_pass(std::make_unique<BitShiftOptimizationPass>());
    add_pass(std::make_unique<ConstFoldOptimizationPass>());
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