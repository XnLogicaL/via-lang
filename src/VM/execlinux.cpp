/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

// Check for Linux
#if defined(__linux__)
#    if !__has_include(<sys/mman.h>)
#        pragma error("viaJIT unavailable: could not find <sys/mman.h>")
#    endif
#    include "common.h"
#    include "execlinux.h"
#    include <sys/mman.h>
#    ifdef __x86_64__
#        include "x86_64codegen.h"
#    elif defined(__aarch64__)
#        include "arm64codegen.h"
#    elif defined(__i386__)
#        include "x86_32codegen.h"
#    else
#        pragma error("viaJIT unavailable: unsupported architecture")
#    endif

namespace via::jit
{

// Initializes a chunk with pre-assembled machine code
void viaJIT_assemblechunk(viaState *, Chunk *chunk, MachineCode code)
{
    // Indicates empty chunk, skip to not waste resources
    if (code.size() == 0)
        return;

    // Create an executable buffer
    void *exec_buf = mmap(nullptr, code.size(), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Check if the buffer failed to allocate
    VIA_ASSERT_SILENT(exec_buf != MAP_FAILED, std::string("viaJIT: mmap failed"));
    std::memcpy(exec_buf, code.data(), code.size());

    chunk->mcode = reinterpret_cast<ExecutableMachineCode>(exec_buf);
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
        MachineCode code = viaJIT_codegen(V, chunk);
        viaJIT_assemblechunk(V, chunk, code);
    }

    // Retrieve and execute chunk machine code
    ExecutableMachineCode mcode_exec = chunk->mcode;
    int exit_code = mcode_exec();

    return exit_code;
}

} // namespace via::jit

#endif