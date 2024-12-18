/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "bytecode.h"

namespace via
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
    int offset = std::strtoul(register_str.substr(1).c_str(), nullptr, 10);

    return offset;
}

viaOperand BytecodeParser::read_operand()
{
    auto lit = consume();

    switch (lit.type)
    {
    case Tokenization::TokenType::LIT_INT:
    case Tokenization::TokenType::LIT_FLOAT:
        return viaOperand{.type = viaOperandType_t::Number, .val_number = std::stod(lit.value)};

    case Tokenization::TokenType::OP_SUB:
        return viaOperand{.type = viaOperandType_t::Number, .val_number = -std::stod(consume().value)};

    case Tokenization::TokenType::LIT_BOOL:
        return viaOperand{.type = viaOperandType_t::Bool, .val_boolean = lit.value == "true"};

    case Tokenization::TokenType::LIT_STRING:
        return viaOperand{.type = viaOperandType_t::String, .val_string = strdup(lit.value.c_str())};

    case Tokenization::TokenType::IDENTIFIER:
        return viaOperand{.type = viaOperandType_t::Register, .val_register = read_register(lit)};

    case Tokenization::TokenType::AT:
        return viaOperand{.type = viaOperandType_t::Identifier, .val_identifier = strdup(consume().value.c_str())};

    default:
        break;
    }

    return {};
}

viaInstruction BytecodeParser::read_instruction()
{
    viaInstruction ins;
    ins.op = read_opcode();
    ins.operandc = 0;

    for (int i = 0; i < 4; ++i)
        ins.operandv[i] = viaOperand{};

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
        instructions.push_back(read_instruction());

    return instructions;
}

} // namespace via
