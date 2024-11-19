/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"

namespace via::Compilation
{

// clang-format off
void Generator::push(std::string line) { src += line; }
void Generator::pushline(std::string line) { push(line + "\n"); }
void Generator::pushinstr(Instruction instr) { instrs.push_back(instr); }
void Generator::free_register(size_t offset) { registers[offset] = false; }
std::string Generator::finalize() { return src; }
// clang-format on

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