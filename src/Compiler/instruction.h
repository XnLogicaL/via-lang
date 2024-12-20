/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "VM/opcode.h"
#include <cstdint>

#ifndef VIA_OPERAND_COUNT
#    define VIA_OPERAND_COUNT 4
#endif

namespace via
{

struct Chunk;
using Register = uint32_t;

// Operand declarations
enum class OperandType : uint8_t
{
    Nil,
    Number,
    Bool,
    String,
    Register,
    Identifier,
};

struct Operand
{
    OperandType type;
    union
    {
        double val_number;
        bool val_boolean;
        const char *val_string;
        const char *val_identifier;
        Register val_register;
    };
};

struct Instruction
{
    OpCode op;
    Operand operand1;
    Operand operand2;
    Operand operand3;
    Chunk *chunk;
};

namespace Compilation
{

Instruction viaC_newinstruction();
Instruction viaC_newinstruction(OpCode, const std::vector<Operand> &);

std::string viaC_compileinstruction(Instruction &);
std::string viaC_compileoperand(Operand &);

bool viaC_checkregister(const Operand &);
bool viaC_checknumber(const Operand &);
bool viaC_checkbool(const Operand &);
bool viaC_checkstring(const Operand &);
bool viaC_checkidentifier(const Operand &);

Operand viaC_newoperand();
Operand viaC_newoperand(double);
Operand viaC_newoperand(bool);
Operand viaC_newoperand(const char *, bool);
Operand viaC_newoperand(Register);

} // namespace Compilation

} // namespace via
