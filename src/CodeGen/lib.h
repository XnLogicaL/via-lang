/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "state.h"
#include "instruction.h"
#include <asmjit/asmjit.h>

namespace via::CodeGen
{

asmjit::Error jitcompileinstruction(asmjit::x86::Assembler &, Instruction &);
asmjit::Imm jittranslateoperand(Operand &);
asmjit::x86::Gp jitgetregister(GPRegister);

} // namespace via::CodeGen