/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "lexer.h"
#include "opcode.h"
#include "token.h"
#include "types.h"

namespace via::VM
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
};

struct Instruction
{
    OpCode op;
    uint8_t operandc;
    Operand operandv[4];
};

class BytecodeParser
{
    Tokenization::Tokenizer *lexer;

    size_t pos;
    std::vector<Tokenization::Token> toks;
    Tokenization::Token consume();
    Tokenization::Token peek(int ahead = 0);

    OpCode read_opcode();
    Register read_register(const Tokenization::Token register_);
    Operand read_operand();
    Instruction read_instruction();

public:
    BytecodeParser(const std::string &src)
        : lexer(new Tokenization::Tokenizer(const_cast<std::string &>(src)))
        , pos(0)
    {
        toks = lexer->tokenize().tokens;
    }
    ~BytecodeParser()
    {
        delete lexer;
    }

    std::vector<Instruction> parse();
};

using RegisterType = Register::RType;
using OperandType  = Operand::OType;

} // namespace via::VM
