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
using GPRegister = uint32_t;

// Operand declarations
enum class OperandType : uint8_t
{
    Nil,
    Number,
    Bool,
    String,
    GPRegister,
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
        GPRegister val_register;
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

Instruction cnewinstruction();
Instruction cnewinstruction(OpCode, const std::vector<Operand> &);

std::string ccompileinstruction(Instruction &);
std::string ccompileoperand(Operand &);

bool ccheckregister(const Operand &);
bool cchecknumber(const Operand &);
bool ccheckbool(const Operand &);
bool ccheckstring(const Operand &);
bool ccheckidentifier(const Operand &);

Operand cnewoperand();
Operand cnewoperand(double);
Operand cnewoperand(bool);
Operand cnewoperand(const char *, bool);
Operand cnewoperand(GPRegister);

} // namespace Compilation

} // namespace via
