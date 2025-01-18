/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "opcode.h"

#ifndef VIA_OPERAND_COUNT
    #define VIA_OPERAND_COUNT 4
#endif

namespace via
{

struct Chunk;

// Operand declarations
enum class OperandType
{
    Nil,
    Number,
    Bool,
    String,
    Register,
};

struct Operand
{
    OperandType type;
    union
    {
        TNumber val_number;
        TBool val_boolean;
        const char *val_string;
        RegId val_register;
    };

    Operand();
    Operand(TNumber);
    Operand(TBool);
    Operand(const char *);
    Operand(RegId);
};

struct Instruction
{
    OpCode op;
    Operand operand1;
    Operand operand2;
    Operand operand3;
    Chunk *chunk;

    Instruction();
    Instruction(OpCode, std::vector<Operand>, Chunk *);
};

std::string to_string(Operand);
std::string to_string(Instruction);

} // namespace via
