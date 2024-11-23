/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "lexer.h"
#include "opcode.h"
#include "token.h"
#include "instruction.h"

namespace via
{

using viaRegister = Compilation::viaRegister;
using viaOperand = Compilation::viaOperand;

class BytecodeParser
{
    Tokenization::Tokenizer *lexer;

    size_t pos;
    std::vector<Tokenization::Token> toks;
    Tokenization::Token consume();
    Tokenization::Token peek(int ahead = 0);

    OpCode read_opcode();
    viaRegister read_register(const Tokenization::Token register_);
    viaOperand read_operand();
    Compilation::viaInstruction read_instruction();

public:
    BytecodeParser(const std::string &src)
        : lexer(new Tokenization::Tokenizer(const_cast<std::string &>(src)))
        , pos(0)
        , toks(lexer->tokenize().tokens)
    {
    }
    ~BytecodeParser()
    {
        delete lexer;
    }

    std::vector<Compilation::viaInstruction> parse();
};

using RegisterType = viaRegister::__type;
using viaOperandType = viaOperand::__type;

} // namespace via
