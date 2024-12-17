/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "constfold.h"

namespace via::Compilation
{

using namespace via::Parsing;
using namespace via::Tokenization;
using namespace AST;

// Folds a constant expression into a constant value
// eg. `1 + 2 * 4` into `9`
void ConstFoldOptimizationPass::fold_constexpr(Generator &gen, ExprNode *expr)
{
    if (!gen.is_constexpr(*expr, 0))
        return;

    gen.evaluate_constexpr(expr);
}

// Folds all possible constants in the AST
void ConstFoldOptimizationPass::apply(Generator &gen, Bytecode &bytecode)
{
#define CHECK(ident, type) type *ident = std::get_if<type>(&stmt)
    for (StmtNode stmt : bytecode.ast->statements)
    {
        if (CHECK(decl_stmt, LocalDeclStmtNode))
            fold_constexpr(gen, &decl_stmt->value.value());
        else if (CHECK(decl_stmt, GlobalDeclStmtNode))
            fold_constexpr(gen, &decl_stmt->value.value());
        else if (CHECK(call_stmt, CallStmtNode))
        {
            fold_constexpr(gen, call_stmt->callee);
            for (ExprNode arg : call_stmt->args)
                fold_constexpr(gen, &arg);
        }
    }
#undef CHECK
}

bool ConstFoldOptimizationPass::is_applicable(const Bytecode &bytecode) const
{
    // Check if the AST has already been optimized
    return bytecode.ast != nullptr;
}

} // namespace via::Compilation
