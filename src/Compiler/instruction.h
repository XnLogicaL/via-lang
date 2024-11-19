/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "VM/opcode.h"

namespace via::Compilation
{

struct Register
{
    enum class RType : uint8_t
    {
        R,  // General Purpose Register
        AR, // Argument Register
        RR, // Return Register
        IR, // Index Register
        ER, // Exit Register
        SR, // Self-argument Register
    };

    RType type;
    uint8_t offset;
};

struct Operand
{
    enum class OType : uint8_t
    {
        Number,
        Bool,
        String,
        Register,
        Identifier
    };

    OType type;
    union
    {
        double num;
        bool boole;
        const char *str;
        const char *ident;
        Register reg;
    };

    const std::string compile() const noexcept;
};

struct Instruction
{
    VM::OpCode op;
    uint8_t operandc;
    Operand operandv[4];

    Instruction(const std::string &op_str, const std::vector<Operand> &operands);
    Instruction()
        : op(VM::OpCode::NOP)
        , operandc(0)
        , operandv()
    {
    }

    const std::string compile() const noexcept;
};

using RegisterType = Register::RType;
using OperandType = Operand::OType;

} // namespace via::Compilation
