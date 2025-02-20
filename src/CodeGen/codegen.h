// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "state.h"
#include <asmjit/asmjit.h>

namespace via::jit {

struct Chunk;
using JitExecutable = int (*)();
JitExecutable generate(Chunk *);

} // namespace via::jit