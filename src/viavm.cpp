#include "common.h"
#include "vm.h"
#include "stack.h"
#include "bytecode.h"
#include "Utils/reader.hpp"
#include "magic_enum.hpp"

using namespace via::VM;

std::string format_operands(const std::vector<Operand>& operands)
{
    std::string result = "[";

    for (const auto& o : operands)
    {
        switch (o.type) {
        case Operand::OType::Bool:
            result += o.boole ? "true" : "false";
            break;
        case Operand::OType::Number:
            result += std::format("{}", o.num);
            break;
        case Operand::OType::Register:
            result += std::format("{}{}", magic_enum::enum_name(o.reg.type), o.reg.offset);
            break;
        case Operand::OType::String:
            result += std::format("\"{}\"", o.str);
            break;
        case Operand::OType::Identifier:
            result += std::format("@{}", o.ident);
            break;
        default:
            result += "unknown";
            break;
        }

        if (!result.empty()) result += ", ";
    }

    result += (operands.size() == 0) ? "]" : "\b\b]";

    return result;
}

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        std::cerr << "Are you fuckin stupid\n";
        return 1;
    }

    auto bytecode = reader::read_file(argv[1]);
    
    BytecodeParser parser(bytecode);
    auto instrs = parser.parse();

    /*for (const auto &instr : instrs)
    {
        std::vector<Operand> operands(instr.operandv, instr.operandv + instr.operandc);

        std::cout << std::format("Instruction(OpCode: {}, Operands: {})\n",
            magic_enum::enum_name(instr.op),
            format_operands(operands));
    }*/

    VirtualMachine vm(instrs);
    vm.init();

    return 0;
}
