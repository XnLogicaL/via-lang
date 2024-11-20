/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "bytecode.h"
#include "common.h"

namespace via::VM
{

Tokenization::Token BytecodeParser::consume()
{
    return toks.at(pos++);
}

Tokenization::Token BytecodeParser::peek(int ahead)
{
    return toks.at(pos + ahead);
}

OpCode BytecodeParser::read_opcode()
{
    return magic_enum::enum_cast<OpCode>(consume().value).value_or(OpCode::NOP);
}

viaRegister BytecodeParser::read_register(const Tokenization::Token register_)
{
    std::string register_str = register_.value;
    std::string register_type;
    size_t pos_ = 0;

    while (pos_ < register_str.length() && !isdigit(register_str.at(pos_)))
    {
        register_type += register_str.at(pos_++);
    }

    int offset = std::strtoul(register_str.substr(pos_).c_str(), nullptr, 10);
    uint8_t uoffset = static_cast<uint8_t>(offset);

    return {.type = magic_enum::enum_cast<RegisterType>(register_type).value_or(RegisterType::R), .offset = uoffset};
}

viaOperand BytecodeParser::read_operand()
{
    auto lit = consume();

    switch (lit.type)
    {
    case Tokenization::TokenType::LIT_INT:
    case Tokenization::TokenType::LIT_FLOAT:
        return viaOperand{.type = viaOperandType::viaNumber, .num = std::stod(lit.value)};

    case Tokenization::TokenType::OP_SUB:
        return viaOperand{.type = viaOperandType::viaNumber, .num = -std::stod(consume().value)};

    case Tokenization::TokenType::LIT_BOOL:
        return viaOperand{.type = viaOperandType::Bool, .boole = lit.value == "true"};

    case Tokenization::TokenType::LIT_STRING:
    case Tokenization::TokenType::LIT_CHAR:
        return viaOperand{.type = viaOperandType::String, .str = strdup(lit.value.c_str())};

    case Tokenization::TokenType::IDENTIFIER:
        return viaOperand{.type = viaOperandType::viaRegister, .reg = read_register(lit)};

    case Tokenization::TokenType::AT:
        return viaOperand{.type = viaOperandType::Identifier, .ident = strdup(consume().value.c_str())};

    default:
        break;
    }

    return {};
}

viaInstruction BytecodeParser::read_instruction()
{
    viaInstruction ins{};
    ins.op = read_opcode();
    ins.operandc = 0;

    for (int i = 0; i < 4; ++i)
    {
        ins.operandv[i] = viaOperand{};
    }

    bool expecting_comma = false;

    while (peek().type != Tokenization::TokenType::SEMICOLON)
    {
        if (expecting_comma)
        {
            consume();
            expecting_comma = false;
            continue;
        }

        ins.operandv[ins.operandc++] = read_operand();
        expecting_comma = true;
    }

    consume();

    return ins;
}

std::vector<viaInstruction> BytecodeParser::parse()
{
    std::vector<viaInstruction> instructions;

    while (pos < toks.size() - 1)
    {
        instructions.push_back(read_instruction());
    }

    return instructions;
}

} // namespace via::VM
