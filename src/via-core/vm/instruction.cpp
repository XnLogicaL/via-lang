// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "instruction.h"
#include <cstring>
#include <iomanip>

namespace via {

enum class Operand : u8 {
  None,
  Generic,
  Register,
  Constant,
  Label,
};

struct Layout {
  const char* opc = NULL;
  Operand a = Operand::None, b = Operand::None, c = Operand::None;
};

struct LayoutPair {
  Opcode opc;
  Layout il;
};

// TODO
static constexpr LayoutPair insn_layout_map[] = {
    {Opcode::NOP, {"nop"}},
    {Opcode::HALT, {"halt"}},
    {Opcode::EXTRAARG1, {"extraarg1", Operand::Generic}},
    {Opcode::EXTRAARG2, {"extraarg2", Operand::Generic, Operand::Generic}},
    {Opcode::EXTRAARG3,
     {"extraarg3", Operand::Generic, Operand::Generic, Operand::Generic}},

};

static char get_operand_prefix(Operand kind) {
  switch (kind) {
    case Operand::Label:
      return 'L';
    case Operand::Register:
      return 'R';
    case Operand::Constant:
      return 'K';
    default:
      break;
  }

  return 0x00;
}

String Instruction::to_string() const {
  const u16 ops[] = {a, b, c};
  const LayoutPair* lp = NULL;

  for (const auto& pair : insn_layout_map) {
    if (pair.opc == op) {
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
    const Operand* ok;
    if ((ok = &(lp->il.a) + i++, *ok == Operand::None))
      break;

    String real_pref;

    char pref;
    if ((pref = get_operand_prefix(*ok), pref == 0))
      real_pref = "";
    else
      real_pref = pref;

    oss << real_pref << std::to_string(operand);

    // another address hack...
    // check if next operand is used and append seperator accordingly
    if (*(ok + 1) != Operand::None)
      oss << " ";
  }

  return oss.str();
}

}  // namespace via
