/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "Utils/reader.h"
#include "bytecode.h"
#include "common.h"
#include "magic_enum.hpp"
#include "stack.h"
#include "vm.h"

#include "libstd/vlstd.h"
#include "libstd/vlmath.h"

using namespace via::VM;

std::string format_operands(const std::vector<Operand>& operands)
{
    std::string result = "[";

    for (const auto& o : operands)
    {
        switch (o.type)
        {
        case OperandType::Bool:
            result += o.boole ? "true" : "false";
            break;
        case OperandType::Number:
            result += std::format("{}", o.num);
            break;
        case OperandType::Register:
            result += std::format("{}{}", magic_enum::enum_name(o.reg.type), o.reg.offset);
            break;
        case OperandType::String:
            result += std::format("\"{}\"", o.str);
            break;
        case OperandType::Identifier:
            result += std::format("@{}", o.ident);
            break;
        default:
            result += "unknown";
            break;
        }

        if (!result.empty())
            result += ", ";
    }

    result += (operands.size() == 0) ? "]" : "\b\b]";

    return result;
}

int main(int argc, char const* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Expected bytecode source\n";
        return 1;
    }

    auto bytecode = FileReader::read_file(argv[1]);

    BytecodeParser parser(bytecode);
    auto instrs = parser.parse();

    /*for (const auto &instr : instrs)
    {
        std::vector<Operand> operands(instr.operandv, instr.operandv + instr.operandc);

        std::cout << std::format("Instruction(OpCode: {}, Operands: {})\n",
            magic_enum::enum_name(instr.op),
            format_operands(operands));
    }*/

    VirtualMachine *vm = new VirtualMachine(instrs);

    viastl::vstl_load(vm);
    viastl::vstl_math_load(vm);

    vm->init();

    return 0;
}
