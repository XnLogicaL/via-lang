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

using viaRegister = Compilation::viaRegister;
using viaOperand = Compilation::viaOperand;
using viaInstruction = Compilation::viaInstruction;

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
    viaInstruction read_instruction();

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

    std::vector<viaInstruction> parse();
};

using RegisterType = viaRegister::RType;
using viaOperandType = viaOperand::OType;

} // namespace via::VM
