/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "codegen.h"
#include "chunk.h"
#include "linux_syscalls.h"

namespace via::jit {

using namespace asmjit;

JitExecutable generate(Chunk *)
{
    JitRuntime rt;
    CodeHolder code;
    code.init(rt.environment(), rt.cpuFeatures());

    // Instantiate sections
    Section *text = code.textSection();
    Section *data = nullptr;

    if (code.newSection(&data, ".data")) {
        return nullptr;
    }

    x86::Assembler a(&code);
    Label L_data = a.newLabel();
    a.section(data);
    a.bind(L_data);
    a.db(0x1);
    a.section(text);

    // Perform compilation

    // Load default exiting behavior
    syscall(a, LxSyscallId::exit, {Imm(0)});

    JitExecutable fn;
    if (rt.add(&fn, &code)) {
        return nullptr;
    }

    return fn;
};

} // namespace via::jit