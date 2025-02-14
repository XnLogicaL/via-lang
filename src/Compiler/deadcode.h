/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "gen.h"
#include "bytecode.h"
#include "optimizer.h"
#include "Parser/ast.h"

namespace via {

class DeadCodeEliminationOptimizationPass : public OptimizationPass {
public:
    void apply(Generator &) override;

private:
    bool always_true(Generator &, IfStmtNode);
    bool always_false(Generator &, IfStmtNode);
    void remove_unreachable_code_in_scope(Generator &, ScopeStmtNode *);
};

} // namespace via