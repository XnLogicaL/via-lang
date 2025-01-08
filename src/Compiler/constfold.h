/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "optimizer.h"
#include "Parser/ast.h"

namespace via
{

class ConstFoldOptimizationPass : public OptimizationPass
{
public:
    void apply(Generator &) override;
    bool is_applicable(Generator &) const override;

private:
    void fold_constexpr(Generator &, ExprNode *);
};

} // namespace via