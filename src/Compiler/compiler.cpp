/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "compiler.h"
#include "constfold.h"

namespace via
{

using namespace via;

// Compiles the AST into instructions
bool Compiler::generate()
{
    // Optimize AST before generating bytecode
    pass_manager.apply_astree(generator);
    generator.generate();
    pass_manager.apply_bytecode(generator);

    return generator.failed;
}

void Compiler::add_default_passes()
{
    // Bitshift optimization
    add_pass(std::make_unique<ConstFoldOptimizationPass>());
}

void Compiler::add_pass(std::unique_ptr<OptimizationPass>)
{
    // pass_manager.add_pass(std::move(pass));
}

} // namespace via