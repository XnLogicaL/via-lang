/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"

using namespace via::Compilation;

void Generator::push(const std::string &line)
{
    src += line;
}

void Generator::pushline(const std::string &line)
{
    src += line + "\n";
}

std::string Generator::finalize()
{
    return src;
}

size_t Generator::get_available_register()
{
    size_t i = 0;

    for (const auto &status : registers)
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

void Generator::free_register(size_t offset)
{
    registers[offset] = false;
    return;
}