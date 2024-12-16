/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "optimizer.h"

namespace via::Compilation
{

// Adds an optimization pass to the manager
void PassManager::add_pass(std::unique_ptr<OptimizationPass> pass)
{
    if (pass->type == PassType::ASTree)
        astree_passes.push_back(std::move(pass));
    else if (pass->type == PassType::Bytecode)
        bytecode_passes.push_back(std::move(pass));
    else
        UNREACHABLE();
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