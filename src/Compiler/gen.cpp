/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"
#include "instruction.h"

namespace via::Compilation
{

void Generator::pushinstr(viaInstruction instr)
{
    instrs.push_back(instr);
}

void Generator::free_register(size_t offset)
{
    registers[offset] = false;
}

std::string Generator::compile()
{
    std::string src;

    for (viaInstruction &instr : instrs)
        src += viaC_compileinstruction(instr) + "\n";

    return src;
}

std::vector<viaInstruction> Generator::get()
{
    return instrs;
}

size_t Generator::get_available_register()
{
    if (registers.size() == 0)
        registers = std::vector<bool>(2048, false);

    size_t i = 0;

    for (bool status : registers)
    {
        if (!status)
        {
            registers[i] = true;
            return i;
        }

        i++;
    }

    // Temp
    VIA_ASSERT(false, "No available registers");
    return 0;
}

size_t Generator::uuid()
{
    return ++uuid_head;
}

} // namespace via::Compilation