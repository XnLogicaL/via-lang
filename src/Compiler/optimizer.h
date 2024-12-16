/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "bytecode.h"
#include "gen.h"
#include "instruction.h"

namespace via::Compilation
{

enum class PassType
{
    ASTree,
    Bytecode,
};

class OptimizationPass
{
public:
    PassType type;

    OptimizationPass() = default;
    virtual ~OptimizationPass() = default;

    virtual void apply(Generator &, Bytecode &) = 0;
    virtual bool is_applicable(const Bytecode &) const
    {
        return true;
    }
};

class PassManager
{
public:
    PassManager() = default;
    ~PassManager() = default;

    void add_pass(std::unique_ptr<OptimizationPass>);
    void apply_all(Generator &, Bytecode &);

private:
    using Passes_t = std::vector<std::unique_ptr<OptimizationPass>>;
    Passes_t astree_passes;
    Passes_t bytecode_passes;
};

} // namespace via::Compilation