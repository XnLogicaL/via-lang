/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

// Check for Linux
#ifdef __linux__
#    if !__has_include(<sys/mman.h>)
#        pragma error("viaJIT unavailable: could not find <sys/mman.h>")
#    endif
#    include "common.h"
#    include "execlinux.h"
#    include <sys/mman.h>
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
    // Indicates empty chunk, skip to not waste resources
    if (mc_size == 0)
        return;

    // Create an executable buffer
    void *exec_buf = mmap(nullptr, mc_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Check if the buffer failed to allocate
    VIA_ASSERT_SILENT(exec_buf != MAP_FAILED, std::string("viaJIT: mmap failed"));
    memcpy(exec_buf, mc_code, mc_size);

    chunk->mcode = reinterpret_cast<MCodeFunc>(exec_buf);
}

// Executes the chunk
int viaJIT_executechunk(viaState *V, Chunk *chunk)
{
    std::cout << std::format("viaJIT: Executing {}\n", reinterpret_cast<const void *>(chunk));
    // Check if the chunk has been compiled before
    if (chunk->mcode == nullptr)
    {
        // Assemble the chunk into machine code
        // Retrieve machine code buffer and buffer size
        auto [mc_buffer, mc_size] = viaJIT_codegen(V, chunk);
        viaJIT_assemblechunk(V, chunk, mc_buffer, mc_size);
    }

    // Retrieve and execute chunk machine code
    MCodeFunc mcode_exec = chunk->mcode;
    int exit_code = mcode_exec(V);

    return exit_code;
}

} // namespace via::jit

#endif