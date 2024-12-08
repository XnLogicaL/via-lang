/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#ifdef __linux__

#include "execlinux.h"
#include <sys/mman.h>

// Ordered from most common to least common
#ifdef __x86_64__
#include "x64codegen.h"
#include "x64assembler.h"
#elif defined(__arm__)
#include "arm64codegen.h"
#include "arm64assembler.h"
// Nobody fucking uses 32-bit anymore...
#elif defined(__i386__)
#include "x32codegen.h"
#include "x32assembler.h"
#endif

namespace via::jit
{

void viaJIT_assemblechunk(viaState *, viaChunk *chunk, unsigned char *mc_code, size_t mc_size)
{
    if (mc_size == 0)
        return;

    void *exec_buf = mmap(nullptr, mc_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    VIA_ASSERT(exec_buf != MAP_FAILED, "viaJIT: mmap failed");
    memcpy(exec_buf, mc_code, mc_size);

    chunk->mcode = reinterpret_cast<viaMCodeExec_t>(exec_buf);
}

void viaJIT_executechunk(viaState *V, viaChunk *chunk)
{
    if (chunk->mcode == nullptr)
    {
        unsigned char *assembly = viaJIT_genasm(V, chunk);
        auto [mc_buffer, mc_size] = viaJIT_assemble(V, assembly);

        viaJIT_assemblechunk(chunk, mc_buffer, mc_size);
    }

    viaMCodeExec_t mcode_exec = chunk->mcode;
    int exit_code = mcode_exec(V);

    return exit_code;
}

} // namespace via::jit

#endif
