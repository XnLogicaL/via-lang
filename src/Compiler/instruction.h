/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "VM/opcode.h"
#include <cstdint>

#ifndef VIA_OPERAND_COUNT
#define VIA_OPERAND_COUNT 4
#endif

namespace via
{

using viaRegister = uint32_t;

// viaOperand declarations
enum class viaOperandType_t : uint8_t
{
    Number,
    Bool,
    String,
    Register,
    Identifier
};

struct viaOperand
{
    viaOperandType_t type;
    union
    {
        double val_number;
        bool val_boolean;
        const char *val_string;
        const char *val_identifier;
        viaRegister val_register;
    };
};

// viaInstruction declarations
using viaInstructionC_t = uint8_t;

struct viaInstruction
{
    bool hot;
    uint16_t pc;
    OpCode op;
    viaInstructionC_t operandc;
    viaOperand operandv[VIA_OPERAND_COUNT];
};

viaInstruction viaC_newinstruction();
viaInstruction viaC_newinstruction(const std::string &, const std::vector<viaOperand> &);

const std::string viaC_compileinstruction(viaInstruction &) noexcept;
const std::string viaC_compileoperand(viaOperand &) noexcept;

} // namespace via
