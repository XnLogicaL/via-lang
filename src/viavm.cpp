/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "Utils/reader.h"
#include "bytecode.h"
#include "common.h"
#include "api.h"
#include "debug.h"
#include "execute.h"
#include "vlrand.h"
#include "state.h"
#include "vlbase.h"
#include "vlmath.h"
#include "vlvec3.h"

using namespace via;

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Expected bytecode source\n";
        return 1;
    }

    std::string bytecode = FileReader::read_file(argv[1]);

    BytecodeParser parser(bytecode);
    std::vector<viaInstruction> instrs = parser.parse();

    viaState *V = viaA_newstate(instrs);

    lib::viaL_loadbaselib(V);

    via_execute(V);

    return 0;
}
