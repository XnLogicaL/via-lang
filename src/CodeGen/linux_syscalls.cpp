/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "linux_syscalls.h"

namespace via
{

using namespace asmjit;

void jitsyscall(x86::Assembler &a, LinuxSyscall syscall, std::vector<asmjit::Operand> ops)
{
    static std::unordered_map<size_t, x86::Gp> arg_map = {
        {0, x86::rdi},
        {1, x86::rsi},
        {2, x86::rdx},
        {3, x86::r9},
        {4, x86::r10},
    };

    // Instantiate syscall id immediate
    Imm syscall_imm = static_cast<uint8_t>(syscall);
    // Move syscall id immediate into rax
    a.mov(x86::rax, syscall_imm);

    size_t i = 0;
    for (const asmjit::Operand &oper : ops)
    {
        // Bound check, linux syscalls have a maximum argc of 5
        if (i > 5)
            break;

        x86::Gp reg = arg_map[i++];
        if (oper.isReg())
            a.mov(reg, oper.as<asmjit::x86::Gp>());
        else if (oper.isMem())
            a.mov(reg, oper.as<asmjit::x86::Mem>());
        else if (oper.isImm())
            a.mov(reg, oper.as<asmjit::Imm>());
    }

    a.syscall();
}

} // namespace via