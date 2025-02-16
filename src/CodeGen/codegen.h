/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "state.h"
#include <asmjit/asmjit.h>

namespace via::jit {

struct Chunk;
using JitExecutable = int (*)();
JitExecutable generate(Chunk *);

} // namespace via::jit