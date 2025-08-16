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

static constexpr LayoutPair insn_layout_map[] = {
    {Opcode::NOP, {"nop"}},
    {Opcode::HALT, {"halt"}},
    {Opcode::EXTRAARG1, {"extraarg1", Operand::Generic}},
    {Opcode::EXTRAARG2, {"extraarg2", Operand::Generic, Operand::Generic}},
    {Opcode::EXTRAARG3,
     {"extraarg3", Operand::Generic, Operand::Generic, Operand::Generic}},
    {Opcode::IADD1, {"iadd1", Operand::Register, Operand::Register}},
    {Opcode::IADD2,
     {"iadd2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::IADD1K, {"iadd1k", Operand::Register, Operand::Constant}},
    {Opcode::IADD2K,
     {"iadd2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FADD1, {"fadd1", Operand::Register, Operand::Register}},
    {Opcode::FADD2,
     {"fadd2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FADD1K, {"fadd1k", Operand::Register, Operand::Constant}},
    {Opcode::FADD2K,
     {"fadd2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FADD1X, {"fadd1x", Operand::Register, Operand::Register}},
    {Opcode::FADD2X,
     {"fadd2x", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FADD1XK, {"fadd1xk", Operand::Register, Operand::Constant}},
    {Opcode::FADD2XK,
     {"fadd2xk", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::ISUB1, {"isub1", Operand::Register, Operand::Register}},
    {Opcode::ISUB2,
     {"isub2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::ISUB1K, {"isub1k", Operand::Register, Operand::Constant}},
    {Opcode::ISUB2K,
     {"isub2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FSUB1, {"fsub1", Operand::Register, Operand::Register}},
    {Opcode::FSUB2,
     {"fsub2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FSUB1K, {"fsub1k", Operand::Register, Operand::Constant}},
    {Opcode::FSUB2K,
     {"fsub2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FSUB1X, {"fsub1x", Operand::Register, Operand::Register}},
    {Opcode::FSUB2X,
     {"fsub2x", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FSUB1XK, {"fsub1xk", Operand::Register, Operand::Constant}},
    {Opcode::FSUB2XK,
     {"fsub2xk", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::IMUL1, {"imul1", Operand::Register, Operand::Register}},
    {Opcode::IMUL2,
     {"imul2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::IMUL1K, {"imul1k", Operand::Register, Operand::Constant}},
    {Opcode::IMUL2K,
     {"imul2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FMUL1, {"fmul1", Operand::Register, Operand::Register}},
    {Opcode::FMUL2,
     {"fmul2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FMUL1K, {"fmul1k", Operand::Register, Operand::Constant}},
    {Opcode::FMUL2K,
     {"fmul2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FMUL1X, {"fmul1x", Operand::Register, Operand::Register}},
    {Opcode::FMUL2X,
     {"fmul2x", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FMUL1XK, {"fmul1xk", Operand::Register, Operand::Constant}},
    {Opcode::FMUL2XK,
     {"fmul2xk", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::IDIV1, {"idiv1", Operand::Register, Operand::Register}},
    {Opcode::IDIV2,
     {"idiv2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::IDIV1K, {"idiv1k", Operand::Register, Operand::Constant}},
    {Opcode::IDIV2K,
     {"idiv2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FDIV1, {"fdiv1", Operand::Register, Operand::Register}},
    {Opcode::FDIV2,
     {"fdiv2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FDIV1K, {"fdiv1k", Operand::Register, Operand::Constant}},
    {Opcode::FDIV2K,
     {"fdiv2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FDIV1X, {"fdiv1x", Operand::Register, Operand::Register}},
    {Opcode::FDIV2X,
     {"fdiv2x", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FDIV1XY, {"fdiv1xy", Operand::Register, Operand::Register}},
    {Opcode::FDIV2XY,
     {"fdiv2xy", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FDIV1XK, {"fdiv1xk", Operand::Register, Operand::Constant}},
    {Opcode::FDIV2XK,
     {"fdiv2xk", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FDIV1XYK, {"fdiv1xyk", Operand::Register, Operand::Constant}},
    {Opcode::FDIV2XYK,
     {"fdiv2xyk", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::IPOW1, {"ipow1", Operand::Register, Operand::Register}},
    {Opcode::IPOW2,
     {"ipow2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::IPOW1K, {"ipow1k", Operand::Register, Operand::Constant}},
    {Opcode::IPOW2K,
     {"ipow2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FPOW1, {"fpow1", Operand::Register, Operand::Register}},
    {Opcode::FPOW2,
     {"fpow2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FPOW1K, {"fpow1k", Operand::Register, Operand::Constant}},
    {Opcode::FPOW2K,
     {"fpow2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FPOW1X, {"fpow1x", Operand::Register, Operand::Register}},
    {Opcode::FPOW2X,
     {"fpow2x", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FPOW1XY, {"fpow1xy", Operand::Register, Operand::Register}},
    {Opcode::FPOW2XY,
     {"fpow2xy", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FPOW1XK, {"fpow1xk", Operand::Register, Operand::Constant}},
    {Opcode::FPOW2XK,
     {"fpow2xk", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FPOW1XYK, {"fpow1xyk", Operand::Register, Operand::Constant}},
    {Opcode::FPOW2XYK,
     {"fpow2xyk", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::IMOD1, {"imod1", Operand::Register, Operand::Register}},
    {Opcode::IMOD2,
     {"imod2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::IMOD1K, {"imod1k", Operand::Register, Operand::Constant}},
    {Opcode::IMOD2K,
     {"imod2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FMOD1, {"fmod1", Operand::Register, Operand::Register}},
    {Opcode::FMOD2,
     {"fmod2", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FMOD1K, {"fmod1k", Operand::Register, Operand::Constant}},
    {Opcode::FMOD2K,
     {"fmod2k", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FMOD1X, {"fmod1x", Operand::Register, Operand::Register}},
    {Opcode::FMOD2X,
     {"fmod2x", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FMOD1XY, {"fmod1xy", Operand::Register, Operand::Register}},
    {Opcode::FMOD2XY,
     {"fmod2xy", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::FMOD1XK, {"fmod1xk", Operand::Register, Operand::Constant}},
    {Opcode::FMOD2XK,
     {"fmod2xk", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::FMOD1XYK, {"fmod1xyk", Operand::Register, Operand::Constant}},
    {Opcode::FMOD2XYK,
     {"fmod2xyk", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::MOVE, {"move", Operand::Register, Operand::Register}},
    {Opcode::XCHG, {"xchg", Operand::Register, Operand::Register}},
    {Opcode::COPY, {"copy", Operand::Register, Operand::Register}},
    {Opcode::COPYREF, {"copyref", Operand::Register, Operand::Register}},
    {Opcode::LOADTRUE, {"loadtrue", Operand::Register}},
    {Opcode::LOADFALSE, {"loadfalse", Operand::Register}},
    {Opcode::NEWSTR, {"newstr", Operand::Register}},
    {Opcode::NEWSTR2, {"newstr2", Operand::Register, Operand::Generic}},
    {Opcode::NEWARR, {"newarr", Operand::Register}},
    {Opcode::NEWARR2, {"newarr2", Operand::Register, Operand::Generic}},
    {Opcode::NEWDICT, {"newdict", Operand::Register}},
    {Opcode::NEWTUPLE, {"newtuple", Operand::Register, Operand::Generic}},
    {Opcode::NEWCLOSURE, {"newclosure", Operand::Register, Operand::Constant}},
    {Opcode::PUSH, {"push", Operand::Register}},
    {Opcode::PUSHK, {"pushk", Operand::Constant}},
    {Opcode::GETARG, {"getarg", Operand::Register, Operand::Generic}},
    {Opcode::GETARGREF, {"getargref", Operand::Register, Operand::Generic}},
    {Opcode::SETARG, {"setarg", Operand::Register, Operand::Generic}},
    {Opcode::GETLOCAL, {"getlocal", Operand::Register, Operand::Generic}},
    {Opcode::GETLOCALREF, {"getlocalref", Operand::Register, Operand::Generic}},
    {Opcode::SETLOCAL, {"setlocal", Operand::Register, Operand::Generic}},
    {Opcode::DUPLOCAL, {"duplocal", Operand::Generic}},
    {Opcode::DUPLOCALREF, {"duplocalref", Operand::Generic}},
    {Opcode::ICASTB, {"icastb", Operand::Register, Operand::Register}},
    {Opcode::ICASTF, {"icastf", Operand::Register, Operand::Register}},
    {Opcode::ICASTSTR, {"icaststr", Operand::Register, Operand::Register}},
    {Opcode::FCASTI, {"fcasti", Operand::Register, Operand::Register}},
    {Opcode::FCASTB, {"fcastb", Operand::Register, Operand::Register}},
    {Opcode::FCASTSTR, {"fcaststr", Operand::Register, Operand::Register}},
    {Opcode::BCASTI, {"bcasti", Operand::Register, Operand::Register}},
    {Opcode::BCASTSTR, {"bcaststr", Operand::Register, Operand::Register}},
    {Opcode::STRCASTI, {"strcasti", Operand::Register, Operand::Register}},
    {Opcode::STRCASTF, {"strcastf", Operand::Register, Operand::Register}},
    {Opcode::STRCASTB, {"strcastb", Operand::Register, Operand::Register}},
    {Opcode::STRCASTARR, {"strcastarr", Operand::Register, Operand::Register}},
    {Opcode::STRCASTDICT,
     {"strcastdict", Operand::Register, Operand::Register}},
    {Opcode::STRCASTFUNC,
     {"strcastfunc", Operand::Register, Operand::Register}},
    {Opcode::CAPTURE, {"capture", Operand::Generic}},
    {Opcode::CALL, {"call", Operand::Register, Operand::Generic}},
    {Opcode::PCALL, {"pcall", Operand::Register, Operand::Generic}},
    {Opcode::RET, {"ret", Operand::Register}},
    {Opcode::RETNIL, {"retnil"}},
    {Opcode::RETTRUE, {"rettrue"}},
    {Opcode::RETFALSE, {"retfalse"}},
    {Opcode::RETK, {"retk", Operand::Constant}},
    {Opcode::GETUPV, {"getupv", Operand::Register, Operand::Generic}},
    {Opcode::GETUPVREF, {"getupvref", Operand::Register, Operand::Generic}},
    {Opcode::SETUPV, {"setupv", Operand::Register, Operand::Generic}},
    {Opcode::STRGET,
     {"strget", Operand::Register, Operand::Register, Operand::Generic}},
    {Opcode::STRSET,
     {"strset", Operand::Register, Operand::Generic, Operand::Generic}},
    {Opcode::STRGETLEN, {"strgetlen", Operand::Register, Operand::Register}},
    {Opcode::STRCONCAT,
     {"strconcat", Operand::Register, Operand::Register, Operand::Register}},
    {Opcode::STRCONCATK,
     {"strconcatk", Operand::Register, Operand::Register, Operand::Constant}},
    {Opcode::ARRGET,
     {"arrget", Operand::Register, Operand::Register, Operand::Generic}},
    {Opcode::ARRSET,
     {"arrset", Operand::Register, Operand::Generic, Operand::Generic}},
    {Opcode::ARRGETLEN, {"arrgetlen", Operand::Register, Operand::Register}},
    {Opcode::DICTGET,
     {"dictget", Operand::Register, Operand::Register, Operand::Generic}},
    {Opcode::DICTSET,
     {"dictset", Operand::Register, Operand::Generic, Operand::Generic}},
    {Opcode::DICTGETLEN, {"dictgetlen", Operand::Register, Operand::Register}},
    {Opcode::NEWINSTANCE,
     {"newinstance", Operand::Register, Operand::Register}},
    {Opcode::GETSUPER, {"getsuper", Operand::Register, Operand::Register}},
    {Opcode::GETSTATIC,
     {"getstatic", Operand::Register, Operand::Register, Operand::Generic}},
    {Opcode::GETDYNAMIC,
     {"getdynamic", Operand::Register, Operand::Register, Operand::Generic}},
    {Opcode::SETSTATIC,
     {"setstatic", Operand::Register, Operand::Register, Operand::Generic}},
    {Opcode::SETDYNAMIC,
     {"setdynamic", Operand::Register, Operand::Register, Operand::Generic}},
    {Opcode::CALLSTATIC,
     {"callstatic", Operand::Register, Operand::Generic, Operand::Generic}},
    {Opcode::PCALLSTATIC,
     {"pcallstatic", Operand::Register, Operand::Generic, Operand::Generic}},
    {Opcode::CALLDYNAMIC,
     {"calldynamic", Operand::Register, Operand::Generic, Operand::Generic}},
    {Opcode::PCALLDYNAMIC,
     {"pcalldynamic", Operand::Register, Operand::Generic, Operand::Generic}},
};

Opcode opcode_from_string(const char* str) {
  for (const auto& pair : insn_layout_map)
    if (strcmp(str, pair.il.opc) == 0)
      return pair.opc;

  return Opcode::NOP;
}

const char* opcode_to_string(Opcode opc) {
  for (const auto& pair : insn_layout_map)
    if (pair.opc == opc)
      return pair.il.opc;

  return "";
}

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
