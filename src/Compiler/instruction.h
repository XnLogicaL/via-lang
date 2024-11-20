/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "VM/opcode.h"

namespace via::Compilation
{

struct viaRegister
{
    enum class RType : uint8_t
    {
        R,  // General Purpose viaRegister
        AR, // Argument viaRegister
        RR, // Return viaRegister
        IR, // Index viaRegister
        ER, // Exit viaRegister
        SR, // Self-argument viaRegister
    };

    RType type;
    uint8_t offset;
};

struct viaOperand
{
    enum class OType : uint8_t
    {
        viaNumber,
        Bool,
        String,
        viaRegister,
        Identifier
    };

    OType type;
    union
    {
        double num;
        bool boole;
        const char *str;
        const char *ident;
        viaRegister reg;
    };

    const std::string compile() const noexcept;
};

struct viaInstruction
{
    VM::OpCode op;
    uint8_t operandc;
    viaOperand operandv[4];

    viaInstruction(const std::string &op_str, const std::vector<viaOperand> &operands);
    viaInstruction()
        : op(VM::OpCode::NOP)
        , operandc(0)
        , operandv()
    {
    }

    const std::string compile() const noexcept;
};

using RegisterType = viaRegister::RType;
using viaOperandType = viaOperand::OType;

} // namespace via::Compilation
