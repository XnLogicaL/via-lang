/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "instruction.h"
#include "opcode.h"

#include <cmath>

namespace via::Compilation
{

// Optimizes division/multiplication operations by changing them to bitshifts if applicable
// ! This is a post-compilation optimization
void optimize_bshift(Compilation::Instruction &instruction) noexcept;

} // namespace via::Compilation