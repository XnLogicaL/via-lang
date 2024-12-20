/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#ifdef __x86_64__
#    include "x86_64codegen.h"

namespace via::jit
{

unsigned char *allocate_machine_code(std::vector<unsigned char> code_vec)
{
    unsigned char *code = new unsigned char[code_vec.size()];
    std::copy(code_vec.begin(), code_vec.end(), code);
    return code;
}

MachineCode viaJIT_codegen(viaState *, Chunk *)
{
    std::vector<unsigned char> code = {
#    ifdef __linux__
        // mov rdi, 0
        0x48,
        0xC7,
        0xC7,
        0x0,
        0x0,
        0x0,
        0x0,
        // mov rax, 60
        0x48,
        0xC7,
        0xC0,
        0x3C,
        0x0,
        0x0,
        0x0,
        // syscall
        0x0F,
        0x05,
#    elif defined(_WIN32) || defined(_WIN64)
        // mov eax, 0 (exit code)
        0xB8,
        0x00,
        0x00,
        0x00,
        0x00,
        // push eax
        0x50,
        // mov eax, [0x12345678]
        0xA1,
        0x78,
        0x56,
        0x34,
        0x12,
        0x00,
        0x00,
        0x00,
        0x00,
        // call eax (call ExitProcess)
        0xFF,
        0xD0,
#    endif
    };

    return {
        allocate_machine_code(code),
        1,
    };
}

} // namespace via::jit

#endif