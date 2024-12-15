/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vexpression.h"

namespace via::Compilation
{

using namespace Parsing;
using namespace AST;
using TokenType = via::Tokenization::TokenType;

// Compile a literal expression
viaRegister compile_lit_expr(Generator *gen, LiteralExprNode lit)
{
    viaRegister reg = gen->get_available_register();
    viaOperand val;

    switch (lit.value.type)
    {
    case TokenType::LIT_FLOAT:
    case TokenType::LIT_INT:
        val = viaC_newoperand(std::stod(lit.value.value));
        break;
    case TokenType::LIT_STRING:
    case TokenType::LIT_CHAR:
    case TokenType::IDENTIFIER:
        val = viaC_newoperand(via_dupstring(lit.value.value), lit.value.type == TokenType::IDENTIFIER);
        break;
    case TokenType::LIT_BOOL:
        val = viaC_newoperand(lit.value.value == "true");
        break;
    case TokenType::LIT_NIL:
        val = viaC_newoperand();
        break;
    default:
        std::cout << "Unsupported literal: " << lit.value.to_string() << "\n";
        break;
    }

    gen->pushinstr(viaC_newinstruction(
        "LOAD",
        {
            viaC_newoperand(reg),
            val,
        }
    ));

    return reg;
}

// Compile a unary expression
viaRegister compile_un_expr(Generator *gen, UnaryExprNode un)
{
    viaRegister expr = compile_expression(gen, *un.expr);
    viaRegister dst = gen->get_available_register();

    gen->free_register(expr);
    gen->pushinstr(viaC_newinstruction(
        "NEG",
        {
            viaC_newoperand(dst),
            viaC_newoperand(expr),
        }
    ));

    return dst;
}

// Compile a binary expression
viaRegister compile_binary_expr(Generator *gen, BinaryExprNode bin)
{
    static std::unordered_map<TokenType, std::string> type_to_op = {
        {TokenType::OP_ADD, "ADD"},
        {TokenType::OP_SUB, "SUB"},
        {TokenType::OP_MUL, "MUL"},
        {TokenType::OP_DIV, "DIV"},
        {TokenType::OP_EXP, "POW"},
        {TokenType::OP_MOD, "MOD"},
        {TokenType::OP_LT, "LT"},
        {TokenType::OP_GT, "GT"},
        {TokenType::OP_EQ, "EQ"},
        {TokenType::OP_NEQ, "NEQ"},
        {TokenType::OP_LEQ, "LEQ"},
        {TokenType::OP_GEQ, "GEQ"},
    };

    viaRegister lhs = compile_expression(gen, *bin.lhs);
    viaRegister rhs = compile_expression(gen, *bin.rhs);
    viaRegister dst = gen->get_available_register();

    std::string op_code = type_to_op[bin.op.type];

    gen->free_register(lhs);
    gen->free_register(rhs);
    gen->pushinstr(viaC_newinstruction(
        op_code,
        {
            viaC_newoperand(dst),
            viaC_newoperand(lhs),
            viaC_newoperand(rhs),
        }
    ));

    return dst;
}

// Compile an index expression
viaRegister compile_index_expr(Generator *gen, IndexExprNode idx)
{
    viaRegister index_of = compile_expression(gen, *idx.object);
    viaRegister index = compile_expression(gen, *idx.index);
    viaRegister dst = gen->get_available_register();

    gen->free_register(index_of);
    gen->free_register(index);
    gen->pushinstr(viaC_newinstruction(
        "LOADIDX",
        {
            viaC_newoperand(dst),
            viaC_newoperand(index_of),
            viaC_newoperand(index),
        }
    ));

    return dst;
}

viaRegister compile_call_expr(Generator *gen, CallExprNode expr)
{
    size_t arg_count = expr.args.size();
    viaRegister callee = compile_expression(gen, *expr.callee);
    viaRegister ret0 = gen->get_available_register();

    // TODO: Add FASTCALL support, by checking the number of arguments
    for (ExprNode argument : expr.args)
    {
        viaRegister arg_reg = compile_expression(gen, argument);
        gen->free_register(arg_reg);
        gen->pushinstr(viaC_newinstruction(
            "PUSHARG",
            {
                viaC_newoperand(arg_reg),
            }
        ));
    }

    gen->pushinstr(viaC_newinstruction(
        "CALL",
        {
            viaC_newoperand(callee),
            viaC_newoperand(static_cast<double>(arg_count)),
        }
    ));

    gen->pushinstr(viaC_newinstruction(
        "POPRET",
        {
            viaC_newoperand(ret0),
        }
    ));

    return ret0;
}

viaRegister compile_var_expr(Generator *gen, VarExprNode expr)
{
    viaRegister dst = gen->get_available_register();

    gen->pushinstr(viaC_newinstruction(
        "LOADVAR",
        {
            viaC_newoperand(dst),
            viaC_newoperand(via_dupstring(expr.ident.value), true),
        }
    ));

    return dst;
}

viaRegister compile_expression(Generator *gen, ExprNode expr)
{
    // Optimize expression by folding it if possible
    // This only works if the expression is a constant expression which doesn't involve runtime
    // optimize_constfold(expr);
    gen->opt_stack.push("prec_constfold");

    if (LiteralExprNode *lit_expr = std::get_if<LiteralExprNode>(&expr))
        return compile_lit_expr(gen, *lit_expr);
    else if (UnaryExprNode *un_expr = std::get_if<UnaryExprNode>(&expr))
        return compile_un_expr(gen, *un_expr);
    else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(&expr))
        return compile_binary_expr(gen, *bin_expr);
    else if (IndexExprNode *idx_expr = std::get_if<IndexExprNode>(&expr))
        return compile_index_expr(gen, *idx_expr);
    else if (CallExprNode *call_expr = std::get_if<CallExprNode>(&expr))
        return compile_call_expr(gen, *call_expr);
    else if (VarExprNode *var_expr = std::get_if<VarExprNode>(&expr))
        return compile_var_expr(gen, *var_expr);

    return -1;
}

} // namespace via::Compilation
