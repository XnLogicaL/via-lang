// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "optimizer.h"

namespace via {

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

void PassManager::apply_astree(Generator &gen)
{
    for (auto &pass : astree_passes) {
        if (pass->is_applicable(gen))
            pass->apply(gen);
    }
}

void PassManager::apply_bytecode(Generator &gen)
{
    for (auto &pass : bytecode_passes) {
        if (pass->is_applicable(gen))
            pass->apply(gen);
    }
}

void PassManager::apply_all(Generator &gen)
{
    apply_astree(gen);
    apply_bytecode(gen);
}

} // namespace via