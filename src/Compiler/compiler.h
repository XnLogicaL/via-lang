/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "common.h"
#include "gen.h"
#include "bytecode.h"
#include "optimizer.h"
#include "cleaner.h"
#include "instruction.h"
#include "bshift.h"
#include "Parser/ast.h"

namespace via::Compilation
{

class Compiler
{
public:
    using Instructions_t = std::vector<viaInstruction>;

    Compiler(Parsing::AST::AST *tree)
        : register_pool(128)
        , register_allocator(register_pool, register_manager)
        , generator(tree, register_pool, register_manager, register_allocator, stack)
    {
    }

    ~Compiler()
    {
        // Invoke cleaner
        cleaner.clean();
        // Cleanup dynamically allocated bytecode object
        delete bytecode.get();
        // Everything else should be automatically cleaned up
    }

    void compile();
    void optimize();
    void add_default_passes();
    void add_pass(std::unique_ptr<OptimizationPass>);
    Instructions_t get();
    const Instructions_t get() const;

private:
    RegisterPool register_pool;
    RegisterManager register_manager;
    RegisterAllocator register_allocator;
    Cleaner cleaner;
    TestStack stack;
    Generator generator;
    PassManager pass_manager;
    std::unique_ptr<Bytecode> bytecode;
};

} // namespace via::Compilation