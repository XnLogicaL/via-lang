/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "optimizer.h"

namespace via::Compilation
{

// Adds an AST optimization pass to the manager
void PassManager::add_astree_pass(std::unique_ptr<OptimizationPass> pass)
{
    astree_passes.push_back(std::move(pass));
}


// Adds a bytecode optimization pass to the manager
void PassManager::add_bytecode_pass(std::unique_ptr<OptimizationPass> pass)
{
    bytecode_passes.push_back(std::move(pass));
}

void PassManager::apply_astree(Generator &gen, Bytecode &bytecode)
{
    for (auto &pass : astree_passes)
    {
        if (pass->is_applicable(bytecode))
            pass->apply(gen, bytecode);
    }
}

void PassManager::apply_bytecode(Generator &gen, Bytecode &bytecode)
{
    for (auto &pass : bytecode_passes)
    {
        if (pass->is_applicable(bytecode))
            pass->apply(gen, bytecode);
    }
}

void PassManager::apply_all(Generator &gen, Bytecode &bytecode)
{
    for (auto &pass : astree_passes)
    {
        if (pass->is_applicable(bytecode))
            pass->apply(gen, bytecode);
    }

    for (auto &pass : bytecode_passes)
    {
        if (pass->is_applicable(bytecode))
            pass->apply(gen, bytecode);
    }
}

} // namespace via::Compilation