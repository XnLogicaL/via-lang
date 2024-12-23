/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"

namespace via::Compilation
{

using namespace Parsing;
using namespace AST;
using namespace Tokenization;

Register Generator::generate_literal_expression(LiteralExprNode lit_expr)
{
    Operand operand = viaC_newoperand();
    Register reg = register_pool.allocate_register();

    switch (lit_expr.value.type)
    {
    case TokenType::LIT_BOOL:
        operand.type = OperandType::Bool;
        operand.val_boolean = (lit_expr.value.value == "true");
        break;
    case TokenType::LIT_STRING:
        operand.type = OperandType::String;
        operand.val_string = via_dupstring(lit_expr.value.value);
        cleaner.add_malloc(operand.val_string); // Track dynamically allocated string
        break;
    case TokenType::LIT_FLOAT:
    case TokenType::LIT_INT:
        operand.type = OperandType::Number;
        operand.val_number = std::stod(lit_expr.value.value);
        break;
    case TokenType::LIT_NIL:
        operand.type = OperandType::Nil;
        break;
    default:
        VIA_ASSERT(false, std::format("Unsupported literal type '{}'", ENUM_NAME(lit_expr.value.type)));
    }

    push_instruction(
        OpCode::LOAD,
        {
            viaC_newoperand(reg),
            operand,
        }
    );

    return reg;
}

Register Generator::generate_unary_expression(UnaryExprNode unary_expr)
{
    Register reg = generate_expression(*unary_expr.expr);
    Register new_reg = register_pool.allocate_register();

    push_instruction(
        OpCode::NEG,
        {
            viaC_newoperand(new_reg),
            viaC_newoperand(reg),
        }
    );

    register_pool.free_register(reg);
    return new_reg;
}

Register Generator::generate_binary_expression(BinaryExprNode bin_expr)
{
    static const std::unordered_map<TokenType, OpCode> operator_map = {
        {TokenType::OP_ADD, OpCode::ADD},
        {TokenType::OP_SUB, OpCode::SUB},
        {TokenType::OP_MUL, OpCode::MUL},
        {TokenType::OP_DIV, OpCode::DIV},
        {TokenType::OP_EXP, OpCode::POW},
        {TokenType::OP_MOD, OpCode::MOD},
        {TokenType::OP_LT, OpCode::LT},
        {TokenType::OP_GT, OpCode::GT},
        {TokenType::OP_EQ, OpCode::EQ},
        {TokenType::OP_NEQ, OpCode::NEQ},
        {TokenType::OP_LEQ, OpCode::LE},
        {TokenType::OP_GEQ, OpCode::GE},
    };

    auto op_it = operator_map.find(bin_expr.op.type);
    VIA_ASSERT(op_it != operator_map.end(), "Unsupported binary operator in generate_binary_expression.");

    OpCode op = op_it->second;
    Register dst = register_pool.allocate_register();
    Register lhs = generate_expression(*bin_expr.lhs);
    Register rhs = generate_expression(*bin_expr.rhs);

    push_instruction(
        op,
        {
            viaC_newoperand(dst),
            viaC_newoperand(lhs),
            viaC_newoperand(rhs),
        }
    );

    register_pool.free_register(lhs);
    register_pool.free_register(rhs);

    return dst;
}

Register Generator::generate_lambda_expression(LambdaExprNode lmd_expr)
{
    Register dst = register_pool.allocate_register();
    push_instruction(OpCode::FUNC, {viaC_newoperand(dst)});

    for (const TypedParamNode &param : lmd_expr.params)
    {
        Register param_reg = register_pool.allocate_register();
        push_instruction(OpCode::POPARG, {viaC_newoperand(param_reg)});
        push_instruction(
            OpCode::SETLOCAL,
            {
                viaC_newoperand(param_reg),
                viaC_newoperand(via_dupstring(param.ident.value), true),
            }
        );
        register_pool.free_register(param_reg);
    }

    for (const StmtNode &lmd_stmt : lmd_expr.body->statements)
        generate_statement(lmd_stmt);

    push_instruction(OpCode::END, {});

    return dst;
}

Register Generator::generate_index_expression(IndexExprNode idx_expr)
{
    Register dst = register_pool.allocate_register();
    Register obj = generate_expression(*idx_expr.object);
    Register idx = generate_expression(*idx_expr.index);

    push_instruction(
        OpCode::LOADIDX,
        {
            viaC_newoperand(dst),
            viaC_newoperand(obj),
            viaC_newoperand(idx),
        }
    );

    register_pool.free_register(obj);
    register_pool.free_register(idx);

    return dst;
}

Register Generator::generate_call_expression(CallExprNode call_expr)
{
    size_t argc = call_expr.args.size();
    Register dst = register_pool.allocate_register();
    Register callee = generate_expression(*call_expr.callee);

    for (const ExprNode &arg : call_expr.args)
    {
        Register arg_reg = generate_expression(arg);
        push_instruction(OpCode::PUSHARG, {viaC_newoperand(arg_reg)});
        register_pool.free_register(arg_reg);
    }

    push_instruction(
        OpCode::CALL,
        {
            viaC_newoperand(callee),
            viaC_newoperand(static_cast<double>(argc)),
        }
    );

    register_pool.free_register(callee);
    push_instruction(OpCode::POPRET, {viaC_newoperand(dst)});

    return dst;
}

Register Generator::generate_variable_expression(VarExprNode var_expr)
{
    Register dst = register_pool.allocate_register();

    push_instruction(
        OpCode::LOADVAR,
        {
            viaC_newoperand(dst),
            viaC_newoperand(via_dupstring(var_expr.ident.value), true),
        }
    );

    return dst;
}

Register Generator::generate_expression(ExprNode expr)
{
    if (LiteralExprNode *lit_expr = std::get_if<LiteralExprNode>(&expr))
        return generate_literal_expression(*lit_expr);
    else if (UnaryExprNode *un_expr = std::get_if<UnaryExprNode>(&expr))
        return generate_unary_expression(*un_expr);
    else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(&expr))
        return generate_binary_expression(*bin_expr);
    else if (LambdaExprNode *lmd_expr = std::get_if<LambdaExprNode>(&expr))
        return generate_lambda_expression(*lmd_expr);
    else if (IndexExprNode *idx_expr = std::get_if<IndexExprNode>(&expr))
        return generate_index_expression(*idx_expr);
    else if (CallExprNode *call_expr = std::get_if<CallExprNode>(&expr))
        return generate_call_expression(*call_expr);
    else if (VarExprNode *var_expr = std::get_if<VarExprNode>(&expr))
        return generate_variable_expression(*var_expr);

    VIA_ASSERT(false, "Unsupported expression type in generate_expression.");
}

} // namespace via::Compilation
