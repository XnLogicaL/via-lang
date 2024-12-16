/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "optimizer.h"
#include "Parser/ast.h"

namespace via::Compilation
{

class ConstFoldOptimizationPass : OptimizationPass
{
public:
    void apply(Generator &, Bytecode &) override;
    bool is_applicable(const Bytecode &) const override;

private:
    void fold_constexpr(Generator &, Parsing::AST::ExprNode *);
    void evaluate_constexpr(Generator &, Parsing::AST::ExprNode *);
};

} // namespace via::Compilation