// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "state.h"
#include "instruction.h"
#include <asmjit/asmjit.h>

#define TOX86GP(reg) x86::Gp::fromTypeAndId(RegType::kX86_KReg, reg)

namespace via::jit {

asmjit::Error compile_instruction(asmjit::x86::Assembler &, Instruction &);
asmjit::Imm translate_operand(Operand &);

} // namespace via::jit