/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"
#include "instruction.h"

#ifndef VIA_CONSTEXPR_MAX_DEPTH
#    define VIA_CONSTEXPR_MAX_DEPTH 5
#endif

namespace via::Compilation
{

using namespace via::Parsing;
using namespace AST;

// Generates bytecode and returns it
std::unique_ptr<Bytecode> Generator::generate()
{
    for (StmtNode stmt : bytecode->ast->statements)
        generate_statement(stmt);

    // Transfer ownership of bytecode object
    return std::move(bytecode);
}

// Returns the successor of iota everytime it's called
size_t Generator::iota()
{
    return ++__iota__;
}

bool Generator::is_constexpr(ExprNode node, int current_depth)
{
    if (current_depth > VIA_CONSTEXPR_MAX_DEPTH)
        // Stack search-depth limit exceeded
        return false;

    if (std::get_if<LiteralExprNode>(&node))
        // Literal is constant
        return true;
    else if (VarExprNode *var_expr = std::get_if<VarExprNode>(&node))
    {
        // Check if the constant has been declared in the stack
        std::optional<ExprNode> const_var = stack.get_constant(var_expr->ident.value);
        if (const_var.has_value())
            // Variable is constant, check further
            return is_constexpr(const_var.value(), current_depth + 1);

        // Variable is not constant
        return false;
    }
    else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(&node))
    {
        // Check both lhs and rhs of the binary expression
        bool lhs_constexpr = is_constexpr(*bin_expr->lhs, current_depth + 1);
        bool rhs_constexpr = is_constexpr(*bin_expr->rhs, current_depth + 1);
        return lhs_constexpr && rhs_constexpr;
    }
    else if (UnaryExprNode *un_expr = std::get_if<UnaryExprNode>(&node))
        return is_constexpr(*un_expr->expr, current_depth + 1);
    else if (GroupExprNode *grp_expr = std::get_if<GroupExprNode>(&node))
        return is_constexpr(*grp_expr->expr, current_depth + 1);

    // Non-constant expression type
    // CallExpr, IndexExpr, etc.
    return false;
}

// Pushes a bytecode instruction
void Generator::push_instruction(OpCode op, std::vector<viaOperand> operands)
{
    bytecode->add_instruction(viaC_newinstruction(op, operands));
}

viaRegister Generator::generate_literal_expression(LiteralExprNode lit_expr)
{
    viaOperand operand = viaC_newoperand();
    viaRegister reg = register_pool.allocate_register();

    switch (lit_expr.value.type)
    {
    case TokenType::LIT_BOOL:
        operand.type = viaOperandType_t::Bool;
        operand.val_boolean = lit_expr.value.value == "true";
        break;
    case TokenType::LIT_CHAR:
    case TokenType::LIT_STRING:
        operand.type = viaOperandType_t::String;
        operand.val_string = via_dupstring(lit_expr.value.value);
        // Add dynamically allocated duplicate string to cleaner
        cleaner.add_malloc(operand.val_string);
        break;
    case TokenType::LIT_FLOAT:
    case TokenType::LIT_INT:
        operand.type = viaOperandType_t::Number;
        operand.val_number = std::stod(lit_expr.value.value);
        break;
    case TokenType::LIT_NIL:
        operand.type = viaOperandType_t::Nil;
        break;
    default:
        break;
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

viaRegister Generator::generate_unary_expression(UnaryExprNode unary_expr)
{
    viaRegister reg = generate_expression(unary_expr);
    viaRegister new_reg = register_pool.allocate_register();

    register_pool.free_register(reg);
    push_instruction(
        OpCode::NEG,
        {
            viaC_newoperand(new_reg),
            viaC_newoperand(reg),
        }
    );

    return reg;
}

viaRegister Generator::generate_binary_expression(BinaryExprNode bin_expr)
{
    static std::unordered_map<TokenType, OpCode> operator_map = {
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

    OpCode op = operator_map[bin_expr.op.type];
    viaRegister dst = register_pool.allocate_register();
    viaRegister lhs = generate_expression(*bin_expr.lhs);
    viaRegister rhs = generate_expression(*bin_expr.rhs);

    register_pool.free_register(lhs);
    register_pool.free_register(rhs);
    push_instruction(
        op,
        {
            viaC_newoperand(dst),
            viaC_newoperand(lhs),
            viaC_newoperand(rhs),
        }
    );

    return lhs;
}

viaRegister Generator::generate_index_expression(IndexExprNode idx_expr) {}

} // namespace via::Compilation