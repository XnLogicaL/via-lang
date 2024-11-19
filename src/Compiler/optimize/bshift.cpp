/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "bshift.h"

namespace via::Compilation
{

void optimize_bshift(Compilation::Instruction &instruction) noexcept
{
    // Threshold for maximum floating point innaccuracy
    static const double epsilon = 1e-9;
    // clang-format off
    static const std::unordered_map<VM::OpCode, VM::OpCode> replacements = {
        {VM::OpCode::MUL, VM::OpCode::BSHL},
        {VM::OpCode::DIV, VM::OpCode::BSHR}
    };
    // clang-format on

    auto opcode_replacement = replacements.find(instruction.op);

    // Early return if the instruction can't be optimized
    if (opcode_replacement == replacements.end())
        return;

    // Handle invalid instruction
    // Technically redundant as the compiler cannot produce illformed instructions
    if (instruction.operandc < 3)
        return;

    double operand_log2 = std::log2(instruction.operandv[2].num);
    bool optimizable_operand_type = (instruction.operandv[2].type == Compilation::OperandType::Number);
    bool optimizable_operand_value = (std::abs(operand_log2 - std::floor(operand_log2)) < epsilon);

    if (optimizable_operand_type && optimizable_operand_value)
    {
        // Replace the opcode with a bit shift opcode
        instruction.op = opcode_replacement->second;
        // Replace the divisor/multiplier with its log base 2
        instruction.operandv[2].num = operand_log2;
    }
}

} // namespace via::Compilation
