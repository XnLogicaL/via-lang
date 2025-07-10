// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vminstr.h"

namespace via {

enum OperandKind : u8 {
  OK_NONE = 0x0,
  OK_GENERIC,
  OK_REGISTER,
  OK_CONSTANT,
  OK_LABEL,
};

struct Layout {
  const char* opc = NULL;
  OperandKind a = OK_NONE, b = OK_NONE, c = OK_NONE;
};

struct LayoutPair {
  Opcode opc;
  Layout il;
};

static constexpr LayoutPair insn_layout_map[] = {
  {VOP_NOP, {"nop"}},
  {VOP_HALT, {"halt"}},
  {VOP_EXTRAARG, {"extraarg", OK_GENERIC, OK_GENERIC, OK_GENERIC}},
};

Opcode opcode_from_string(const char* str) {
  for (const auto& pair : insn_layout_map)
    if (strcmp(str, pair.il.opc))
      return pair.opc;

  return VOP_NOP;
}

String opcode_to_string(Opcode opc) {
  for (const auto& pair : insn_layout_map)
    if (pair.opc == opc)
      return pair.il.opc;

  return "";
}

static char get_operand_prefix(OperandKind kind) {
  switch (kind) {
  case OK_LABEL:
    return 'L';
  case OK_REGISTER:
    return 'R';
  case OK_CONSTANT:
    return 'K';
  default:
    break;
  }

  return 0x00;
}

String instruction_format(Instruction insn) {
  const u16 ops[] = {insn.a, insn.b, insn.c};
  const LayoutPair* lp = NULL;

  for (const auto& pair : insn_layout_map) {
    if (pair.opc == insn.op) {
      lp = &pair;
      break;
    }
  }

  if (lp == NULL)
    return "<unmapped-instruction>";

  std::ostringstream oss;
  oss << std::left << std::setfill(' ') << std::setw(20);
  oss << lp->il.opc;

  for (size_t i = 0; u16 operand : ops) {
    // disgusting addressing hack
    const OperandKind* ok = &(lp->il.a) + i++;

    // no other operand should be used after
    if (*ok == OK_NONE)
      break;

    String real_pref;

    char pref = get_operand_prefix(*ok);
    if (pref == 0x00)
      real_pref = "";
    else
      real_pref = pref;

    oss << real_pref << std::to_string(operand);

    // another address hack...
    // check if next operand is used and append seperator accordingly
    if (*(ok + 1) != OK_NONE)
      oss << " ";
  }

  return oss.str();
}

} // namespace via
