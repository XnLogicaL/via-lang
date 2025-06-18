// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_INSTRUCTION_H
#define VIA_INSTRUCTION_H

#include "common.h"
#include "vmopc.h"

#include "sbits.h"
#include "color.h"

namespace via {

namespace vm {

inline constexpr size_t OPERAND_INVALID = 0xFFFF;

struct InstructionData {
  std::string comment = "";
};

struct Instruction {
  Opcode op = VOP_NOP;
  uint16_t a, b, c;
};

} // namespace vm

} // namespace via

#endif
