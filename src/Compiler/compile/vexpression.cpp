/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vexpression.h"

using namespace via::Parsing;

namespace via::Compilation
{

using TokenType = Tokenization::TokenType;

// Helper to create an viaOperand for a register
inline viaOperand make_register_operand(RegisterType reg_type, size_t reg_num)
{
    return viaOperand{.type = viaOperandType::Register, .reg = viaRegister(reg_type, reg_num)};
}

// Helper to create an instruction
inline viaInstruction make_instruction(OpCode opcode, std::initializer_list<viaOperand> operands)
{
    viaInstruction instr;
    instr.op = opcode;
    instr.operandc = operands.size();
    std::copy(operands.begin(), operands.end(), instr.operandv);
    return instr;
}

// Helper to map TokenType to OpCode
inline OpCode map_token_to_opcode(TokenType type)
{
    static const std::unordered_map<TokenType, OpCode> op_table = {
        {TokenType::OP_ADD, OpCode::ADD},
        {TokenType::OP_SUB, OpCode::SUB},
        {TokenType::OP_MUL, OpCode::MUL},
        {TokenType::OP_DIV, OpCode::DIV},
        {TokenType::OP_MOD, OpCode::MOD},
        {TokenType::OP_EQ, OpCode::EQ},
        {TokenType::OP_EXP, OpCode::POW},
        {TokenType::OP_GEQ, OpCode::GE},
        {TokenType::OP_GT, OpCode::GT},
        {TokenType::OP_LEQ, OpCode::LE},
        {TokenType::OP_LT, OpCode::LT},
        {TokenType::OP_NEQ, OpCode::NEQ}
    };

    auto it = op_table.find(type);
    if (it == op_table.end())
    {
        throw std::runtime_error("Unsupported binary operator type");
    }
    return it->second;
}

inline void load_arguments(Generator *gen, std::vector<AST::ExprNode *> args)
{
    size_t i = 0;
    for (AST::ExprNode *arg : args)
    {
        // Load argument into a gp register
        size_t expr_reg = compile_expression(gen, *arg);
        // Since we're gonna move it anyways, mark it as free
        gen->free_register(expr_reg);
        // clang-format off
        // Load argument into an ar register
        gen->pushinstr(make_instruction(
            OpCode::MOV,
            {// Corresponding argument register
                make_register_operand(RegisterType::AR, i++),
                // Since there are no `get_available_register` calls in between, we can safely use `expr_reg` again
                make_register_operand(RegisterType::R, expr_reg)
            }
        ));
        // clang-format on
    }
}

// Compile a literal expression
size_t compile_lit_expr(Generator *gen, AST::LitExprNode lit)
{
    size_t reg = gen->get_available_register();

    viaOperand dst_o = make_register_operand(RegisterType::R, reg);
    viaOperand value_o;

    viaInstruction instr;
    instr.op = OpCode::LI;

    // Determine the type of literal and populate the instruction
    switch (lit.val.type)
    {
    case TokenType::LIT_INT:
    {
        value_o = viaOperand{.type = viaOperandType::Number, .num = std::stod(lit.val.value)};
        break;
    }
    case TokenType::LIT_FLOAT:
    {
        value_o = viaOperand{.type = viaOperandType::Number, .num = std::stod(lit.val.value)};
        break;
    }
    case TokenType::LIT_STRING:
    {
        value_o = viaOperand{.type = viaOperandType::String, .str = strdup(lit.val.value.c_str())};
        break;
    }
    case TokenType::LIT_BOOL:
    {
        value_o = viaOperand{.type = viaOperandType::Bool, .boole = lit.val.value == "true"};
        break;
    }
    default:
        value_o = viaOperand{.type = viaOperandType::Number, .num = 0.0f};
        break;
    }

    // Populate instruction operands
    instr.operandc = 2; // Destination register and literal value
    instr.operandv[0] = dst_o;
    instr.operandv[1] = value_o;

    // Emit the instruction
    gen->pushinstr(instr);

    return reg;
}

// Compile a unary expression
size_t compile_un_expr(Generator *gen, AST::UnExprNode un)
{
    size_t expr_reg = compile_expression(gen, un);
    size_t un_reg = gen->get_available_register();

    viaInstruction instr =
        make_instruction(OpCode::NEG, {make_register_operand(RegisterType::R, expr_reg), make_register_operand(RegisterType::R, un_reg)});

    gen->free_register(expr_reg);
    gen->pushinstr(instr);
    return un_reg;
}

// Compile a binary expression
size_t compile_binary_expr(Generator *gen, AST::BinExprNode bin)
{
    size_t lhs_reg = compile_expression(gen, *bin.lhs);
    size_t rhs_reg = compile_expression(gen, *bin.rhs);
    size_t bin_reg = gen->get_available_register();

    OpCode op_code = map_token_to_opcode(bin.op.type);

    // clang-format off
    viaInstruction instr = make_instruction(
        op_code,
        {
            make_register_operand(RegisterType::R, bin_reg),
            make_register_operand(RegisterType::R, lhs_reg),
            make_register_operand(RegisterType::R, rhs_reg)
        }
    );
    // clang-format on
    // Optimize binary expression if possible
    optimize_bshift(instr);
    gen->opt_stack.push("postc_bshift");

    gen->pushinstr(instr);
    gen->free_register(lhs_reg);
    gen->free_register(rhs_reg);

    return bin_reg;
}

// Compile an index expression
size_t compile_index_expr(Generator *gen, AST::IndexExprNode idx)
{
    size_t tbl_reg = gen->get_available_register();
    size_t idx_reg = gen->get_available_register();

    auto ident = std::get<AST::IdentExprNode>(*idx.ident).val;

    // clang-format off
    gen->pushinstr(make_instruction(
        OpCode::LOADLOCAL, 
        {
            viaOperand{.type = viaOperandType::Identifier, .ident = strdup(ident.value.c_str())},
            make_register_operand(RegisterType::R, tbl_reg)
        }
    ));

    gen->pushinstr(make_instruction(
        OpCode::LI,
        {
            make_register_operand(RegisterType::IR, 0),
            viaOperand{.type = viaOperandType::String, .str = strdup(idx.index.value.c_str())}
        }
    ));

    gen->pushinstr(make_instruction(
        OpCode::LOADIDX,
        {
            make_register_operand(RegisterType::R, idx_reg),
            make_register_operand(RegisterType::R, tbl_reg)
        }
    ));
    // clang-format on

    gen->free_register(tbl_reg);
    return idx_reg;
}

size_t compile_call_expr(Generator *gen, Parsing::AST::CallExprNode expr)
{
    // clang-format off
    // Load function call arguments into their respective registers
    load_arguments(gen, expr.args);
    size_t func_reg = gen->get_available_register();

    gen->pushinstr(make_instruction(
        OpCode::LOADLOCAL,
        {
            viaOperand{.type = viaOperandType::Identifier, .ident = strdup(std::get<AST::IdentExprNode>(*expr.ident).val.value.c_str())},
            make_register_operand(RegisterType::R, func_reg)
        }
    ));

    gen->pushinstr(make_instruction(
        OpCode::CALL,
        {
            make_register_operand(RegisterType::R, func_reg)
        }
    ));

    size_t ret_reg = gen->get_available_register();
    // Move the return value to the allocated register
    gen->pushinstr(make_instruction(
        OpCode::MOV,
        {
            make_register_operand(RegisterType::R, ret_reg),
            make_register_operand(RegisterType::RR, 0)
        }
    ));
    // clang-format on

    return ret_reg;
}

size_t compile_ident_expr(Generator *gen, Parsing::AST::IdentExprNode expr)
{
    size_t expr_reg = gen->get_available_register();

    // clang-format off
    gen->pushinstr(make_instruction(
        OpCode::LOADLOCAL,
        {
            viaOperand{.type = viaOperandType::Identifier, .ident = strdup(expr.val.value.c_str())},
            make_register_operand(RegisterType::R, expr_reg)
        }
    ));
    // clang-format on

    return expr_reg;
}

size_t compile_expression(Generator *gen, AST::ExprNode expr)
{
    // Optimize expression by folding it if possible
    // This only works if the expression is a constant expression which doesn't involve runtime
    optimize_constfold(expr);
    gen->opt_stack.push("prec_constfold");

    if (auto lit_expr = std::get_if<AST::LitExprNode>(&expr))
        return compile_lit_expr(gen, *lit_expr);
    else if (auto un_expr = std::get_if<AST::UnExprNode>(&expr))
        return compile_un_expr(gen, *un_expr);
    else if (auto grp_expr = std::get_if<AST::GroupExprNode>(&expr))
        return compile_expression(gen, *grp_expr->expr);
    else if (auto bin_expr = std::get_if<AST::BinExprNode>(&expr))
        return compile_binary_expr(gen, *bin_expr);
    else if (auto lambda_expr = std::get_if<AST::LambdaExprNode>(&expr))
        // TODO: ...
        return 0;
    else if (auto call_expr = std::get_if<AST::CallExprNode>(&expr))
        return compile_call_expr(gen, *call_expr);
    else if (auto index_expr = std::get_if<AST::IndexExprNode>(&expr))
        return compile_index_expr(gen, *index_expr);
    else if (auto index_call_expr = std::get_if<AST::IndexCallExprNode>(&expr))
    {
        AST::IndexExprNode idx_expr;
        idx_expr.index = index_call_expr->index;
        idx_expr.ident = index_call_expr->ident;

        size_t index_reg = compile_index_expr(gen, idx_expr);
        // Load call arguments into their respective registers
        load_arguments(gen, index_call_expr->args);
        // clang-format off
        gen->pushinstr(make_instruction(
            OpCode::CALL,
            {
                make_register_operand(RegisterType::R, index_reg)
            }
        ));
        // clang-format on
        gen->free_register(index_reg);
    }
    else if (auto ident_expr = std::get_if<AST::IdentExprNode>(&expr))
        return compile_ident_expr(gen, *ident_expr);

    return -1;
}

} // namespace via::Compilation
