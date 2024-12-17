/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "gen.h"
#include "bytecode.h"
#include "optimizer.h"
#include "Parser/ast.h"

namespace via::Compilation
{

class DeadCodeEliminationOptimizationPass : public OptimizationPass
{
public:
    void apply(Generator &, Bytecode &) override;

private:
    bool always_true(Generator &, Parsing::AST::IfStmtNode);
    bool always_false(Generator &, Parsing::AST::IfStmtNode);
    void remove_unreachable_code_in_scope(Generator &, Parsing::AST::ScopeStmtNode *);
};

} // namespace via::Compilation