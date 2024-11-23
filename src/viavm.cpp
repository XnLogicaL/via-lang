/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "Utils/reader.h"
#include "bytecode.h"
#include "common.h"
#include "api.h"
#include "execute.h"
#include "libstd/vlrand.h"
#include "state.h"
#include "libstd/vlbase.h"
#include "libstd/vlmath.h"
#include "libstd/vlvec3.h"

using namespace via;
using namespace Compilation;

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

    viaState *V = via::via_newstate(instrs);
    lib::viaL_loadbaselib(V);
    lib::viaL_loadmathlib(V);
    lib::viaL_loadvec3lib(V);
    lib::viaL_loadrandlib(V);
    via_execute(V);

    return 0;
}
