/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "codegen.h"
#include "chunk.h"
#include "linux_syscalls.h"

namespace via::CodeGen
{

using namespace asmjit;

JITFunc jitgenerate(Chunk *chunk)
{
    Error g_err;
    JitRuntime rt;
    CodeHolder code;
    code.init(rt.environment(), rt.cpuFeatures());

    // Instantiate sections
    Section *text = code.textSection();
    Section *data = nullptr;

    g_err = code.newSection(&data, ".data");
    if (g_err)
        return nullptr;

    // Instantiate assembler
    x86::Assembler a(&code);
    // Initialize .data section
    Label L_data = a.newLabel();
    a.section(data);
    a.bind(L_data);
    a.db(0x1);
    a.section(text);

    // Perform compilation

    // Load default exiting behavior
    jitsyscall(a, LinuxSyscall::exit, {Imm(0)});

    JITFunc fn;
    g_err = rt.add(&fn, &code);
    if (g_err)
        return nullptr;

    return fn;
};

} // namespace via::CodeGen