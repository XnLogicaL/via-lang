/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

// Check for Windows
#if defined(_WIN32) || defined(_WIN64) || defined(IS_WSL)
#    include "common.h"
#    include "core.h"
#    include "shared.h"
#    include "chunk.h"
#    include "state.h"

namespace via::jit
{

void viaJIT_assemblechunk(viaState *, Chunk *, unsigned char *, size_t);
int viaJIT_executechunk(viaState *, Chunk *);

} // namespace via::jit

#endif