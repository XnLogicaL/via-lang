/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "encoder.h"

#define INSTR_SEP 0xff
#define OPERAND_SEP 0xfe

namespace via
{

char Encoder::encode_opcode(OpCode op)
{
    return static_cast<char>(op);
}

std::vector<char> Encoder::encode_operand(Operand oper)
{
    std::vector<char> encoding;

    // Encode type (just the type value for now)
    encoding.push_back(static_cast<char>(oper.type));

    switch (oper.type)
    {
    case OperandType::Bool:
    case OperandType::Register:
        // For Bool, we store the boolean value. For Register, we store the register number
        encoding.push_back(static_cast<char>(oper.type == OperandType::Bool ? oper.val_boolean : oper.val_register));
        break;

    case OperandType::String:
    {
        // For strings and identifiers, encode them as a sequence of characters (null-terminated)
        const char *ptr = oper.val_string;
        while (*ptr)
            encoding.push_back(*ptr++);
        // Optionally add a null-terminator to the end of the string/identifier
        encoding.push_back('\0');
        break;
    }

    case OperandType::Nil:
        encoding.push_back(0x0);
        break;

    case OperandType::Number:
        // For a 64-bit float (e.g., `double`), we need to encode it as bytes
        {
            const char *ptr = reinterpret_cast<const char *>(&oper.val_number);
            encoding.insert(encoding.end(), ptr, ptr + sizeof(oper.val_number));
        }
        break;
    }

    return encoding;
}

std::vector<char> Encoder::encode(std::vector<Instruction> instrs)
{
    std::vector<char> encoding;

    for (Instruction instr : instrs)
    {
        std::vector<char> encoded_operand1 = encode_operand(instr.operand1);
        std::vector<char> encoded_operand2 = encode_operand(instr.operand2);
        std::vector<char> encoded_operand3 = encode_operand(instr.operand3);

        encoding.push_back(INSTR_SEP);
        encoding.push_back(encode_opcode(instr.op));
        encoding.push_back(OPERAND_SEP);
        encoding.insert(encoding.end(), encoded_operand1.begin(), encoded_operand1.end());
        encoding.push_back(OPERAND_SEP);
        encoding.insert(encoding.end(), encoded_operand2.begin(), encoded_operand2.end());
        encoding.push_back(OPERAND_SEP);
        encoding.insert(encoding.end(), encoded_operand3.begin(), encoded_operand3.end());
        encoding.push_back(OPERAND_SEP);
        encoding.push_back(INSTR_SEP);
    }

    encoding.push_back(INSTR_SEP);
    return encoding;
}

OpCode Encoder::decode_opcode(char op)
{
    return static_cast<OpCode>(op);
}

Operand Encoder::decode_operand(std::vector<char>::const_iterator it)
{
    Operand operand;
    operand.type = static_cast<OperandType>(*it++);

    switch (operand.type)
    {
    case OperandType::Bool:
        operand.val_boolean = static_cast<bool>(*it++);
        break;
    case OperandType::Register:
        operand.val_register = static_cast<RegId>(*it++);
        break;
    case OperandType::String:
    {
        std::string temp;
        while (*it != '\0')
            temp.push_back(*it++);
        ++it; // Skip null terminator
        char *buf = new char[temp.size() + 1];
        std::strcpy(buf, temp.c_str());
        operand.val_string = buf;
        break;
    }
    case OperandType::Number:
    {
        double num;
        std::memcpy(&num, &(*it), sizeof(double));
        operand.val_number = num;
        it += sizeof(double);
        break;
    }
    default:
        break;
    }

    return operand;
}

std::vector<Instruction> Encoder::decode(std::vector<char> encoding)
{
    std::vector<Instruction> instructions;
    for (auto it = encoding.begin(); it != encoding.end();)
    {
        if (static_cast<unsigned char>(*it) == INSTR_SEP)
        {
            ++it; // Skip instruction separator
            Instruction instr;
            instr.op = decode_opcode(*it++);
            if (static_cast<unsigned char>(*it) == OPERAND_SEP)
                ++it; // Skip operand separator

            instr.operand1 = decode_operand(it);
            if (static_cast<unsigned char>(*it) == OPERAND_SEP)
                ++it; // Skip operand separator

            instr.operand2 = decode_operand(it);
            if (static_cast<unsigned char>(*it) == OPERAND_SEP)
                ++it; // Skip operand separator

            instr.operand3 = decode_operand(it);
            if (static_cast<unsigned char>(*it) == OPERAND_SEP)
                ++it; // Skip operand separator

            instructions.push_back(instr);
        }
    }

    return instructions;
}

} // namespace via
