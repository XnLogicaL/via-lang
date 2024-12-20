/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#ifdef __x86_64__
#    include "common.h"
#    include "state.h"
#    include "chunk.h"

namespace via::jit
{

struct MachineCode
{
    unsigned char *machine_code;
    size_t machine_code_len;
};

MachineCode viaJIT_codegen(viaState *, Chunk *);

} // namespace via::jit

#endif