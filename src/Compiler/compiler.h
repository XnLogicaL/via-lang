// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "common.h"
#include "gen.h"
#include "bytecode.h"
#include "optimizer.h"
#include "cleaner.h"
#include "instruction.h"
#include "Parser/ast.h"

namespace via {

class Compiler {
public:
    using Instructions_t = std::vector<Instruction>;

    Compiler(ProgramData *program)
        : program(program)
        , generator(program)
    {
    }

    ~Compiler()
    {
        // Invoke cleaner
        cleaner.clean();
        // Everything else should be automatically cleaned up
    }

    bool generate();
    void add_default_passes();
    void add_pass(std::unique_ptr<OptimizationPass>);

private:
    ProgramData *program;
    Cleaner cleaner;
    Generator generator;
    PassManager pass_manager;
};

} // namespace via