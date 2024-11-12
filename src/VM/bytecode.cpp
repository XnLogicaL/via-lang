#include "common.h"
#include "bytecode.h"

#include "magic_enum.hpp" // Include for magic_enum

using namespace via::VM;
using namespace via::Tokenization;

Token BytecodeParser::consume()
{
    return toks.at(pos++);
}

Token BytecodeParser::peek(int ahead)
{
    return toks.at(pos + ahead);
}

OpCode BytecodeParser::read_opcode()
{
    return magic_enum::enum_cast<OpCode>(consume().value)
        .value_or(OpCode::NOP);
}

Register BytecodeParser::read_register(const Token register_)
{
    std::string register_str = register_.value;
    std::string register_type;
    size_t pos_ = 0;

    while (pos_ < register_str.length()
        && !isdigit(register_str.at(pos_)))
    {
        register_type += register_str.at(pos_++);
    }

    return Register{
        .type = magic_enum::enum_cast<Register::RType>(register_type).value_or(Register::RType::R),
        .offset = static_cast<uint8_t>(std::strtoul(register_str.substr(pos_).c_str(), nullptr, 10))
    };
}

Operand BytecodeParser::read_operand()
{
    auto lit = consume();

    switch (lit.type)
    {
    case TokenType::LIT_INT:
    case TokenType::LIT_FLOAT:
        return Operand {
            .type = Operand::OType::Number,
            .num = std::stod(lit.value)
        };
    
    case TokenType::LIT_BOOL:
        return Operand {
            .type = Operand::OType::Bool,
            .boole = lit.value == "true"
        };
    
    case TokenType::LIT_STRING:
    case TokenType::LIT_CHAR:
        return Operand {
            .type = Operand::OType::String,
            .str = strdup(lit.value.c_str())
        };

    case TokenType::IDENTIFIER:
        return Operand{
            .type = Operand::OType::Register,
            .reg = read_register(lit)
        };

    case TokenType::AT:
        return Operand {
            .type = Operand::OType::Identifier,
            .ident = strdup(consume().value.c_str())
        };

    default:
        break;
    }

    return {};
}

Instruction BytecodeParser::read_instruction()
{
    Instruction ins{};
    ins.op = read_opcode();
    ins.operandc = 0;

    for (int i = 0; i < 4; ++i)
    {
        ins.operandv[i] = Operand{};
    }

    bool expecting_comma = false;

    while (peek().type != TokenType::SEMICOLON)
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

std::vector<Instruction> BytecodeParser::parse()
{
    std::vector<Instruction> instructions;

    while (pos < toks.size() - 1)
    {
        instructions.push_back(read_instruction());
    }
    
    return instructions;
}