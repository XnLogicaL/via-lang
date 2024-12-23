/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#if defined(_WIN32) || defined(_WIN64) || defined(IS_WSL)
#    include "execwin.h"
#    include "Windows.h"
// Ordered from most common to least common
#    ifdef __x86_64__
#        include "x86_64codegen.h"
#    elif defined(__aarch64__)
#        include "arm64codegen.h"
// Nobody fucking uses 32-bit anymore...
#    elif defined(__i386__)
#        include "x86_32codegen.h"
#    endif
// Yes, ARM32 isn't here because guess what... NOBODY FUCKING USES IT!
// Be grateful i386 is even supported in the first place
namespace via::jit
{

// Initializes a chunk with pre-assembled machine code
void viaJIT_assemblechunk(viaState *, Chunk *chunk, unsigned char *mc_code, size_t mc_size)
{
    if (mc_size == 0)
        return;

    void *exec_buf = VirtualAlloc(NULL, mc_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // Check if the buffer failed to allocate
    VIA_ASSERT(exec_buf != NULL, "viaJIT: VirtualAlloc failed");
    memcpy(exec_buf, mc_code, mc_size);

    chunk->mcode = reinterpret_cast<ExecutableMachineCode>(exec_buf);
}

// Executes the chunk
int viaJIT_executechunk(viaState *V, Chunk *chunk)
{
    // Check if the chunk has been compiled before
    if (chunk->mcode == nullptr)
    {
        // Assemble the chunk into machine code
        // Retrieve machine code buffer and buffer size
        auto [mc_buffer, mc_size] = viaJIT_codegen(V, chunk);

        // Initialize chunk
        viaJIT_assemblechunk(V, chunk, mc_buffer, mc_size);
    }

    // Retrieve and execute chunk machine code
    ExecutableMachineCode mcode_exec = chunk->mcode;
    int exit_code = mcode_exec(V);

    return exit_code;
}

} // namespace via::jit

#endif