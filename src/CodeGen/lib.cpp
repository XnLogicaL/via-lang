/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "lib.h"

#define TOX86GP(reg) x86::Gp::fromTypeAndId(RegType::kX86_KReg, reg)

namespace via::CodeGen
{

using namespace asmjit;

Imm jittranslateoperand(Operand &oper)
{
    switch (oper.type)
    {
    case OperandType::Bool:
        return Imm(oper.val_boolean);
    case OperandType::Number:
        return Imm(oper.val_number);
    case OperandType::String:
        return Imm(oper.val_string);
    default:
        return Imm();
    }
}

Error jitcompileinstruction(x86::Assembler &a, Instruction &instruction)
{
    switch (instruction.op)
    {
    case OpCode::END:
    case OpCode::NOP:
        return a.nop(); // nop
    case OpCode::CPY:
    case OpCode::MOV:
    {
        x86::Gp dst = TOX86GP(instruction.operand1.val_register);
        x86::Gp src = TOX86GP(instruction.operand2.val_register);
        return a.mov(dst, src); // mov dst, src
    }
    case OpCode::LOAD:
    {
        x86::Gp dst = TOX86GP(instruction.operand1.val_register);
        Imm val = jittranslateoperand(instruction.operand2);
        return a.mov(dst, val); // mov dst, imm
    }
    case OpCode::SWAP:
    {
        x86::Gp dst = TOX86GP(instruction.operand1.val_register);
        x86::Gp src = TOX86GP(instruction.operand2.val_register);
        return a.xchg(dst, src); // xchg dst, src
    }
    case OpCode::PUSH:
        return a.sub(x86::rsp, 16); // sub rsp, 16
    case OpCode::POP:
        return a.add(x86::rsp, 16); // add rsp, 16
    case OpCode::ADD:
    {
        x86::Gp dst = TOX86GP(instruction.operand1.val_register);
        x86::Gp lhs = TOX86GP(instruction.operand2.val_register);
        x86::Gp rhs = TOX86GP(instruction.operand3.val_register);
        a.add(lhs, rhs);        // add lhs, rhs
        return a.mov(dst, lhs); // mov dst, lhs
    }
    default:
        break;
    }

    return a.nop(); // nop
}

} // namespace via::CodeGen