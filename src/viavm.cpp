/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "Utils/reader.h"
#include "bytecode.h"
#include "common.h"
#include "api.h"
#include "execute.h"

#include "libstd/vlstd.h"
#include "state.h"

using namespace via;
using namespace VM;

std::string format_operands(const std::vector<viaOperand> &operands)
{
    std::string result = "[";

    for (const auto &o : operands)
    {
        switch (o.type)
        {
        case viaOperandType::Bool:
            result += o.boole ? "true" : "false";
            break;
        case viaOperandType::viaNumber:
            result += std::format("{}", o.num);
            break;
        case viaOperandType::viaRegister:
            result += std::format("{}{}", magic_enum::enum_name(o.reg.type), o.reg.offset);
            break;
        case viaOperandType::String:
            result += std::format("\"{}\"", o.str);
            break;
        case viaOperandType::Identifier:
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

int main(int argc, char const *argv[])
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
        std::vector<viaOperand> operands(instr.operandv, instr.operandv +
    instr.operandc);

        std::cout << std::format("viaInstruction(OpCode: {}, Operands: {})\n",
            magic_enum::enum_name(instr.op),
            format_operands(operands));
    }*/

    // Spawn a new thread
    viaState *V = via::via_newstate(instrs);
    lib::vstl_load(V);
    via_execute(V);

    return 0;
}
