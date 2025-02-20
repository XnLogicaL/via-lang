// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "compiler.h"
#include "constfold.h"
#include "types.h"

namespace via {

// Compiles the AST into instructions
bool Compiler::generate()
{
    // Optimize AST before generating bytecode
    pass_manager.apply_astree(generator);
    generator.generate();
    pass_manager.apply_bytecode(generator);

    for (const TValue &constant : generator.constants) {
        program->constants->push_back(constant.clone());
    }

    return false;
}

void Compiler::add_default_passes() {}

void Compiler::add_pass(std::unique_ptr<OptimizationPass>) {}

} // namespace via