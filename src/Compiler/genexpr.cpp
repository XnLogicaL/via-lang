/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"

namespace via::Compilation
{

using namespace Parsing;
using namespace AST;
using namespace Tokenization;

GPRegister Generator::generate_literal_expression(LiteralExprNode lit_expr)
{
    Operand operand = cnewoperand();
    GPRegister reg = register_pool.allocate_register();

    switch (lit_expr.value.type)
    {
    case TokenType::LIT_BOOL:
        operand.type = OperandType::Bool;
        operand.val_boolean = (lit_expr.value.value == "true");
        break;
    case TokenType::LIT_STRING:
        operand.type = OperandType::String;
        operand.val_string = dupstring(lit_expr.value.value);
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
            cnewoperand(reg),
            operand,
        }
    );

    return reg;
}

GPRegister Generator::generate_unary_expression(UnaryExprNode unary_expr)
{
    GPRegister reg = generate_expression(*unary_expr.expr);
    GPRegister new_reg = register_pool.allocate_register();

    push_instruction(
        OpCode::NEG,
        {
            cnewoperand(new_reg),
            cnewoperand(reg),
        }
    );

    register_pool.free_register(reg);
    return new_reg;
}

GPRegister Generator::generate_binary_expression(BinaryExprNode bin_expr)
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
    GPRegister dst = register_pool.allocate_register();
    GPRegister lhs = generate_expression(*bin_expr.lhs);
    GPRegister rhs = generate_expression(*bin_expr.rhs);

    push_instruction(
        op,
        {
            cnewoperand(dst),
            cnewoperand(lhs),
            cnewoperand(rhs),
        }
    );

    register_pool.free_register(lhs);
    register_pool.free_register(rhs);

    return dst;
}

GPRegister Generator::generate_lambda_expression(LambdaExprNode lmd_expr)
{
    GPRegister dst = register_pool.allocate_register();
    push_instruction(OpCode::FUNC, {cnewoperand(dst)});

    for (const TypedParamNode &param : lmd_expr.params)
    {
        GPRegister param_reg = register_pool.allocate_register();
        push_instruction(OpCode::POPARG, {cnewoperand(param_reg)});
        push_instruction(
            OpCode::SETLOCAL,
            {
                cnewoperand(param_reg),
                cnewoperand(dupstring(param.ident.value), true),
            }
        );
        register_pool.free_register(param_reg);
    }

    for (const StmtNode &lmd_stmt : lmd_expr.body->statements)
        generate_statement(lmd_stmt);

    push_instruction(OpCode::END, {});

    return dst;
}

GPRegister Generator::generate_index_expression(IndexExprNode idx_expr)
{
    GPRegister dst = register_pool.allocate_register();
    GPRegister obj = generate_expression(*idx_expr.object);
    GPRegister idx = generate_expression(*idx_expr.index);

    push_instruction(
        OpCode::LOADIDX,
        {
            cnewoperand(dst),
            cnewoperand(obj),
            cnewoperand(idx),
        }
    );

    register_pool.free_register(obj);
    register_pool.free_register(idx);

    return dst;
}

GPRegister Generator::generate_call_expression(CallExprNode call_expr)
{
    size_t argc = call_expr.args.size();
    GPRegister dst = register_pool.allocate_register();
    GPRegister callee = generate_expression(*call_expr.callee);

    for (const ExprNode &arg : call_expr.args)
    {
        GPRegister arg_reg = generate_expression(arg);
        push_instruction(OpCode::PUSHARG, {cnewoperand(arg_reg)});
        register_pool.free_register(arg_reg);
    }

    push_instruction(
        OpCode::CALL,
        {
            cnewoperand(callee),
            cnewoperand(static_cast<double>(argc)),
        }
    );

    register_pool.free_register(callee);
    push_instruction(OpCode::POPRET, {cnewoperand(dst)});

    return dst;
}

GPRegister Generator::generate_variable_expression(VarExprNode var_expr)
{
    GPRegister dst = register_pool.allocate_register();

    push_instruction(
        OpCode::LOADVAR,
        {
            cnewoperand(dst),
            cnewoperand(dupstring(var_expr.ident.value), true),
        }
    );

    return dst;
}

GPRegister Generator::generate_expression(ExprNode expr)
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
