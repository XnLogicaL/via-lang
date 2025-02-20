// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "lib.h"

namespace via::jit {

using namespace asmjit;

Imm translate_operand(Operand &oper)
{
    switch (oper.type) {
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

Error compile_instruction(x86::Assembler &a, Instruction &instruction)
{
    switch (instruction.op) {
    case OpCode::NOP:
        return a.nop(); // nop
    case OpCode::MOVE: {
        x86::Gp dst = TOX86GP(instruction.operand0.val_register);
        x86::Gp src = TOX86GP(instruction.operand1.val_register);
        return a.mov(dst, src); // mov dst, src
    }
    case OpCode::SWAP: {
        x86::Gp dst = TOX86GP(instruction.operand0.val_register);
        x86::Gp src = TOX86GP(instruction.operand1.val_register);
        return a.xchg(dst, src); // xchg dst, src
    }
    case OpCode::PUSH:
        return a.sub(x86::rsp, 16); // sub rsp, 16
    case OpCode::POP:
        return a.add(x86::rsp, 16); // add rsp, 16
    case OpCode::ADD: {
        x86::Gp lhs = TOX86GP(instruction.operand0.val_register);
        x86::Gp rhs = TOX86GP(instruction.operand1.val_register);
        return a.add(lhs, rhs); // add lhs, rhs
    }
    default:
        break;
    }

    return a.nop(); // nop
}

} // namespace via::jit