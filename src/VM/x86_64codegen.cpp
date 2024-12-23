/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#ifdef __x86_64__
#    include "x86_64codegen.h"

namespace via::jit
{

std::vector<unsigned char> viaJIT_codegen(viaState *, Chunk *)
{
    // clang-format off
    std::vector<unsigned char> code = {
        0xbf, 0x00, 0x00, 0x00, 0x00,
        0xb8, 0x3c, 0x00, 0x00, 0x00,
        0x0f, 0x05,
    }; // clang-format on

    return code;
}

} // namespace via::jit

#endif