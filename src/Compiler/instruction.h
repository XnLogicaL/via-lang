/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "VM/opcode.h"
#include <cstdint>

namespace via::Compilation
{

struct viaRegister
{
    using __offset = uint8_t;
    enum class __type : uint8_t
    {
        R,  // General Purpose viaRegister
        AR, // Argument viaRegister
        RR, // Return viaRegister
    };

    __type type;
    __offset offset;
};

struct viaOperand
{
    enum class __type : uint8_t
    {
        Number,
        Bool,
        String,
        Register,
        Identifier
    };

    __type type;
    union
    {
        double num;
        bool boole;
        const char *str;
        const char *ident;
        viaRegister reg;
    };
};

struct viaInstruction
{
    OpCode op;
    uint8_t operandc;
    viaOperand operandv[4];
};

viaInstruction viaC_newinstruction();
viaInstruction viaC_newinstruction(const std::string &op_str, const std::vector<viaOperand> &operands);

const std::string viaC_compileinstruction(viaInstruction &) noexcept;
const std::string viaC_compileoperand(viaOperand &) noexcept;

using viaRegisterType = viaRegister::__type;
using viaRegisterOffset = viaRegister::__offset;
using viaOperandType = viaOperand::__type;

} // namespace via::Compilation
