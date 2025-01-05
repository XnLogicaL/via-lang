/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "bshift.h"

namespace via::Compilation
{

void BitShiftOptimizationPass::apply(Generator &, Bytecode &bytecode)
{
    for (Instruction &instr : bytecode.get())
    {
        if (instr.op != OpCode::MULRR && instr.op != OpCode::DIVRR)
            continue;

        // Check if the operand is a power of 2
        bool is_number = instr.operand1.type == OperandType::Number;
        if (!is_number)
            continue;

        std::optional<double> power_of_2 = is_power_of_2(instr.operand1.val_number);
        if (!power_of_2.has_value())
            continue;

        // Replace with bit-shift instructions
        if (instr.op == OpCode::MULRR)
        {
            instr.op = OpCode::BSHLRR;                        // Bitwise shift left
            instr.operand1 = cnewoperand(power_of_2.value()); // Use the shift count
        }
        else if (instr.op == OpCode::DIVRR)
        {
            instr.op = OpCode::BSHRRR;                        // Bitwise shift right
            instr.operand1 = cnewoperand(power_of_2.value()); // Use the shift count
        }
    }
}

std::optional<double> BitShiftOptimizationPass::is_power_of_2(double value)
{
    if (value <= 0.0 || std::fmod(value, 1.0) != 0.0)
        // Not a positive integer
        return std::nullopt;

    double log2_val = std::log2(value);
    if (std::floor(log2_val) == log2_val)
        return log2_val;

    // Not a power of 2
    return std::nullopt;
}

} // namespace via::Compilation
