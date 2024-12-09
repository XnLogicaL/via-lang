/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#if defined(_WIN32) || defined(_WIN64)
#    include "execwin.h"
#    include "Windows.h"
// Ordered from most common to least common
#    ifdef __x86_64__
#        include "x86_64codegen.h"
#        include "x86_64assembler.h"
#    elif defined(__aarch64__)
#        include "arm64codegen.h"
#        include "arm64assembler.h"
// Nobody fucking uses 32-bit anymore...
#    elif defined(__i386__)
#        include "x86_32codegen.h"
#        include "x86_32assembler.h"
#    endif
// Yes, ARM32 isn't here because guess what... NOBODY FUCKING USES IT!
// Be grateful i386 is even supported in the first place
namespace via::jit
{

// Initializes a chunk with pre-assembled machine code
void viaJIT_assemblechunk(viaState *, viaChunk *chunk, unsigned char *mc_code, size_t mc_size)
{
    if (mc_size == 0)
        return;

    void *exec_buf = VirtualAlloc(NULL, mc_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // Check if the buffer failed to allocate
    VIA_ASSERT(exec_buf != NULL, "viaJIT: VirtualAlloc failed");
    memcpy(exec_buf, mc_code, mc_size);

    chunk->mcode = reinterpret_cast<viaMCodeExec_t>(exec_buf);
}

// Executes the chunk
int viaJIT_executechunk(viaState *V, viaChunk *chunk)
{
    // Check if the chunk has been compiled before
    if (chunk->mcode == nullptr)
    {
        // Generate assembly as string
        char *assembly = viaJIT_genasm(V, chunk);
        // Assemble the chunk into machine code
        // Retrieve machine code buffer and buffer size
        auto [mc_buffer, mc_size] = viaJIT_assemble(V, assembly);

        // Initialize chunk
        viaJIT_assemblechunk(chunk, mc_buffer, mc_size);
    }

    // Retrieve and execute chunk machine code
    viaMCodeExec_t mcode_exec = chunk->mcode;
    int exit_code = mcode_exec(V);

    return exit_code;
}

} // namespace via::jit

#endif