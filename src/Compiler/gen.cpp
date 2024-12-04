/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"
#include "instruction.h"

namespace via::Compilation
{

// clang-format off
void Generator::pushinstr(viaInstruction instr) { instrs.push_back(instr); }
void Generator::free_register(size_t offset) { registers[offset] = false; }
// clang-format on

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
    size_t i = 0;

    for (const bool &status : registers)
    {
        if (status == false)
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

} // namespace via::Compilation