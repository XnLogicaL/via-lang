/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "state.h"
#include <asmjit/asmjit.h>

namespace via
{

struct Chunk;

namespace CodeGen
{

using JITFunc = int (*)();
JITFunc jitgenerate(Chunk *);

} // namespace CodeGen

} // namespace via