/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"
#include "instruction.h"
#include <cmath>

#ifndef VIA_CONSTEXPR_MAX_DEPTH
    #define VIA_CONSTEXPR_MAX_DEPTH 5
#endif

namespace via::Compilation
{

using namespace via::Parsing;
using namespace AST;

template<typename T, typename K>
using BinaryEvaluator = std::function<K(T, T)>;

// Utility function for checking if an expression holds a number literal
bool is_number_literal(LiteralExprNode lit_expr)
{
    return lit_expr.value.type == TokenType::LIT_INT || lit_expr.value.type == TokenType::LIT_FLOAT;
}

// Utility function for checking if an operator type is an arithmetic operator
bool is_arithmetic_operator(TokenType op)
{
    return op == TokenType::OP_ADD || op == TokenType::OP_SUB || op == TokenType::OP_MUL || op == TokenType::OP_MOD || op == TokenType::OP_EXP;
}

// Utility function for checking if an operator type is a comparison operator
bool is_comparison_operator(TokenType op)
{
    return op == TokenType::OP_EQ || op == TokenType::OP_NEQ;
}

// Utility function for checking if an operator type is a numeric comparsion
// operator
bool is_numeric_comparison_operator(TokenType op)
{
    return op == TokenType::OP_LEQ || op == TokenType::OP_GEQ || op == TokenType::OP_LT || op == TokenType::OP_GT;
}

// Utility function to check if two token types are compatible for comparison
bool are_compatible_types(TokenType lhs_type, TokenType rhs_type)
{
    return (lhs_type == TokenType::LIT_INT && rhs_type == TokenType::LIT_FLOAT) ||
           (lhs_type == TokenType::LIT_FLOAT && rhs_type == TokenType::LIT_INT) || lhs_type == rhs_type; // Same types are always compatible
}

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
    static size_t __iota__ = 0;
    return ++__iota__;
}

bool Generator::is_constexpr(ExprNode, int)
{
    return false;
}

void Generator::evaluate_constexpr(ExprNode *)
{
    return;
}

// Pushes a bytecode instruction
void Generator::push_instruction(OpCode op = OpCode::NOP, std::vector<Operand> operands = {})
{
    Instruction instruction = cnewinstruction(op, operands);

    if (initialize_with_chunk)
    {
        initialize_with_chunk = false;
        instruction.chunk = current_chunk;
    }

    bytecode->add_instruction(instruction);
}

Operand Generator::generate_operand(LiteralExprNode lit_expr)
{
    Operand operand = cnewoperand();
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

    return operand;
}

TValue Generator::generate_tvalue(LiteralExprNode lit_expr)
{
    switch (lit_expr.value.type)
    {
    case TokenType::LIT_INT:
    case TokenType::LIT_FLOAT:
        return stackvalue(nullptr, std::stod(lit_expr.value.value));
    case TokenType::LIT_BOOL:
        return stackvalue(nullptr, lit_expr.value.value == "true");
    case TokenType::LIT_STRING:
        return stackvalue(nullptr, newstring(nullptr, lit_expr.value.value.c_str()));
    default:
        return stackvalue(nullptr);
    }
}

GPRegister Generator::allocate_temp_register()
{
    GPRegister reg = allocate_register();
    free_register(reg);
    return reg;
}

GPRegister Generator::allocate_register()
{
    for (auto it : register_pool)
        if (it.second == false)
        {
            register_pool[it.first] = false;
            return it.first;
        }

    return VIA_GPREGISTER_INVALID;
}

void Generator::free_register(GPRegister reg)
{
    register_pool[reg] = true;
}

void Generator::load_operand(Operand dst, Operand operand)
{
    static const std::unordered_map<OperandType, OpCode> load_op = {
        {OperandType::Nil, OpCode::LOADNIL},
        {OperandType::Bool, OpCode::LOADBOOL},
        {OperandType::Number, OpCode::LOADNUMBER},
        {OperandType::String, OpCode::LOADSTRING}
    };

    auto it = load_op.find(operand.type);
    if (it != load_op.end())
        push_instruction(it->second, {dst, operand});
}

} // namespace via::Compilation