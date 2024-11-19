/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "lexer.h"
#include "opcode.h"
#include "token.h"
#include "types.h"
#include "instruction.h"

#include "Compiler/instruction.h"

namespace via::VM
{

using Register = Compilation::Register;
using Operand = Compilation::Operand;
using Instruction = Compilation::Instruction;

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
using OperandType = Operand::OType;

} // namespace via::VM
