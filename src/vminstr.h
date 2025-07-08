// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_INSTRUCTION_H
#define VIA_INSTRUCTION_H

#include "common.h"
#include "bitutils.h"

namespace via {

enum Opcode : uint16_t {
  VOP_NOP,

  VOP_IADDII,
  VOP_IADDIIK,
  VOP_FADDIF,
  VOP_FADDIFK,
  VOP_FADDFIK,
  VOP_FADDFF,
  VOP_FADDFFK,

  VOP_MOV,
  VOP_LOADK,
  VOP_LOADNIL,
  VOP_LOADINT,
  VOP_LOADFP,
  VOP_LOADTRUE,
  VOP_LOADFALSE,
};

struct InstructionData {
  String comment = "";
};

struct Instruction {
  Opcode op = VOP_NOP;
  uint16_t a, b, c;
};

} // namespace via

#endif
