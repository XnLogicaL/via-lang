// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "instruction.h"
#include <cstring>
#include <iomanip>

namespace via {

namespace core {

namespace vm {

enum class Operand : u8 {
  None = 0x0,
  Generic,
  Register,
  Constant,
  Label,
};

#define OK_NONE     Operand::None
#define OK_GENERIC  Operand::Generic
#define OK_REGISTER Operand::Register
#define OK_CONSTANT Operand::Constant
#define OK_LABEL    Operand::Label

struct Layout {
  const char* opc = NULL;
  Operand a = OK_NONE, b = OK_NONE, c = OK_NONE;
};

struct LayoutPair {
  Opcode opc;
  Layout il;
};

static constexpr LayoutPair insn_layout_map[] = {
  {Opcode::NOP, {"nop"}},
  {Opcode::HALT, {"halt"}},
  {Opcode::EXTRAARG1, {"extraarg1", OK_GENERIC}},
  {Opcode::EXTRAARG2, {"extraarg2", OK_GENERIC, OK_GENERIC}},
  {Opcode::EXTRAARG3, {"extraarg3", OK_GENERIC, OK_GENERIC, OK_GENERIC}},
  {Opcode::IADD1, {"iadd1", OK_REGISTER, OK_REGISTER}},
  {Opcode::IADD2, {"iadd2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::IADD1K, {"iadd1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::IADD2K, {"iadd2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FADD1, {"fadd1", OK_REGISTER, OK_REGISTER}},
  {Opcode::FADD2, {"fadd2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FADD1K, {"fadd1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FADD2K, {"fadd2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FADD1X, {"fadd1x", OK_REGISTER, OK_REGISTER}},
  {Opcode::FADD2X, {"fadd2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FADD1XK, {"fadd1xk", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FADD2XK, {"fadd2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::ISUB1, {"isub1", OK_REGISTER, OK_REGISTER}},
  {Opcode::ISUB2, {"isub2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::ISUB1K, {"isub1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::ISUB2K, {"isub2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FSUB1, {"fsub1", OK_REGISTER, OK_REGISTER}},
  {Opcode::FSUB2, {"fsub2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FSUB1K, {"fsub1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FSUB2K, {"fsub2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FSUB1X, {"fsub1x", OK_REGISTER, OK_REGISTER}},
  {Opcode::FSUB2X, {"fsub2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FSUB1XK, {"fsub1xk", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FSUB2XK, {"fsub2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::IMUL1, {"imul1", OK_REGISTER, OK_REGISTER}},
  {Opcode::IMUL2, {"imul2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::IMUL1K, {"imul1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::IMUL2K, {"imul2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FMUL1, {"fmul1", OK_REGISTER, OK_REGISTER}},
  {Opcode::FMUL2, {"fmul2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FMUL1K, {"fmul1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FMUL2K, {"fmul2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FMUL1X, {"fmul1x", OK_REGISTER, OK_REGISTER}},
  {Opcode::FMUL2X, {"fmul2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FMUL1XK, {"fmul1xk", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FMUL2XK, {"fmul2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::IDIV1, {"idiv1", OK_REGISTER, OK_REGISTER}},
  {Opcode::IDIV2, {"idiv2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::IDIV1K, {"idiv1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::IDIV2K, {"idiv2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FDIV1, {"fdiv1", OK_REGISTER, OK_REGISTER}},
  {Opcode::FDIV2, {"fdiv2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FDIV1K, {"fdiv1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FDIV2K, {"fdiv2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FDIV1X, {"fdiv1x", OK_REGISTER, OK_REGISTER}},
  {Opcode::FDIV2X, {"fdiv2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FDIV1XY, {"fdiv1xy", OK_REGISTER, OK_REGISTER}},
  {Opcode::FDIV2XY, {"fdiv2xy", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FDIV1XK, {"fdiv1xk", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FDIV2XK, {"fdiv2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FDIV1XYK, {"fdiv1xyk", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FDIV2XYK, {"fdiv2xyk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::IPOW1, {"ipow1", OK_REGISTER, OK_REGISTER}},
  {Opcode::IPOW2, {"ipow2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::IPOW1K, {"ipow1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::IPOW2K, {"ipow2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FPOW1, {"fpow1", OK_REGISTER, OK_REGISTER}},
  {Opcode::FPOW2, {"fpow2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FPOW1K, {"fpow1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FPOW2K, {"fpow2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FPOW1X, {"fpow1x", OK_REGISTER, OK_REGISTER}},
  {Opcode::FPOW2X, {"fpow2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FPOW1XY, {"fpow1xy", OK_REGISTER, OK_REGISTER}},
  {Opcode::FPOW2XY, {"fpow2xy", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FPOW1XK, {"fpow1xk", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FPOW2XK, {"fpow2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FPOW1XYK, {"fpow1xyk", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FPOW2XYK, {"fpow2xyk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::IMOD1, {"imod1", OK_REGISTER, OK_REGISTER}},
  {Opcode::IMOD2, {"imod2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::IMOD1K, {"imod1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::IMOD2K, {"imod2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FMOD1, {"fmod1", OK_REGISTER, OK_REGISTER}},
  {Opcode::FMOD2, {"fmod2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FMOD1K, {"fmod1k", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FMOD2K, {"fmod2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FMOD1X, {"fmod1x", OK_REGISTER, OK_REGISTER}},
  {Opcode::FMOD2X, {"fmod2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FMOD1XY, {"fmod1xy", OK_REGISTER, OK_REGISTER}},
  {Opcode::FMOD2XY, {"fmod2xy", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::FMOD1XK, {"fmod1xk", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FMOD2XK, {"fmod2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::FMOD1XYK, {"fmod1xyk", OK_REGISTER, OK_CONSTANT}},
  {Opcode::FMOD2XYK, {"fmod2xyk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::MOVE, {"move", OK_REGISTER, OK_REGISTER}},
  {Opcode::XCHG, {"xchg", OK_REGISTER, OK_REGISTER}},
  {Opcode::COPY, {"copy", OK_REGISTER, OK_REGISTER}},
  {Opcode::COPYREF, {"copyref", OK_REGISTER, OK_REGISTER}},
  {Opcode::LOADTRUE, {"loadtrue", OK_REGISTER}},
  {Opcode::LOADFALSE, {"loadfalse", OK_REGISTER}},
  {Opcode::NEWSTR, {"newstr", OK_REGISTER}},
  {Opcode::NEWSTR2, {"newstr2", OK_REGISTER, OK_GENERIC}},
  {Opcode::NEWARR, {"newarr", OK_REGISTER}},
  {Opcode::NEWARR2, {"newarr2", OK_REGISTER, OK_GENERIC}},
  {Opcode::NEWDICT, {"newdict", OK_REGISTER}},
  {Opcode::NEWTUPLE, {"newtuple", OK_REGISTER, OK_GENERIC}},
  {Opcode::NEWCLOSURE, {"newclosure", OK_REGISTER, OK_CONSTANT}},
  {Opcode::PUSH, {"push", OK_REGISTER}},
  {Opcode::PUSHK, {"pushk", OK_CONSTANT}},
  {Opcode::GETARG, {"getarg", OK_REGISTER, OK_GENERIC}},
  {Opcode::GETARGREF, {"getargref", OK_REGISTER, OK_GENERIC}},
  {Opcode::SETARG, {"setarg", OK_REGISTER, OK_GENERIC}},
  {Opcode::GETLOCAL, {"getlocal", OK_REGISTER, OK_GENERIC}},
  {Opcode::GETLOCALREF, {"getlocalref", OK_REGISTER, OK_GENERIC}},
  {Opcode::SETLOCAL, {"setlocal", OK_REGISTER, OK_GENERIC}},
  {Opcode::DUPLOCAL, {"duplocal", OK_GENERIC}},
  {Opcode::DUPLOCALREF, {"duplocalref", OK_GENERIC}},
  {Opcode::ICASTB, {"icastb", OK_REGISTER, OK_REGISTER}},
  {Opcode::ICASTF, {"icastf", OK_REGISTER, OK_REGISTER}},
  {Opcode::ICASTSTR, {"icaststr", OK_REGISTER, OK_REGISTER}},
  {Opcode::FCASTI, {"fcasti", OK_REGISTER, OK_REGISTER}},
  {Opcode::FCASTB, {"fcastb", OK_REGISTER, OK_REGISTER}},
  {Opcode::FCASTSTR, {"fcaststr", OK_REGISTER, OK_REGISTER}},
  {Opcode::BCASTI, {"bcasti", OK_REGISTER, OK_REGISTER}},
  {Opcode::BCASTSTR, {"bcaststr", OK_REGISTER, OK_REGISTER}},
  {Opcode::STRCASTI, {"strcasti", OK_REGISTER, OK_REGISTER}},
  {Opcode::STRCASTF, {"strcastf", OK_REGISTER, OK_REGISTER}},
  {Opcode::STRCASTB, {"strcastb", OK_REGISTER, OK_REGISTER}},
  {Opcode::STRCASTARR, {"strcastarr", OK_REGISTER, OK_REGISTER}},
  {Opcode::STRCASTDICT, {"strcastdict", OK_REGISTER, OK_REGISTER}},
  {Opcode::STRCASTFUNC, {"strcastfunc", OK_REGISTER, OK_REGISTER}},
  {Opcode::CAPTURE, {"capture", OK_GENERIC}},
  {Opcode::CALL, {"call", OK_REGISTER, OK_GENERIC}},
  {Opcode::PCALL, {"pcall", OK_REGISTER, OK_GENERIC}},
  {Opcode::RET, {"ret", OK_REGISTER}},
  {Opcode::RETNIL, {"retnil"}},
  {Opcode::RETTRUE, {"rettrue"}},
  {Opcode::RETFALSE, {"retfalse"}},
  {Opcode::RETK, {"retk", OK_CONSTANT}},
  {Opcode::GETUPV, {"getupv", OK_REGISTER, OK_GENERIC}},
  {Opcode::GETUPVREF, {"getupvref", OK_REGISTER, OK_GENERIC}},
  {Opcode::SETUPV, {"setupv", OK_REGISTER, OK_GENERIC}},
  {Opcode::STRGET, {"strget", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {Opcode::STRSET, {"strset", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {Opcode::STRGETLEN, {"strgetlen", OK_REGISTER, OK_REGISTER}},
  {Opcode::STRCONCAT, {"strconcat", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {Opcode::STRCONCATK, {"strconcatk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {Opcode::ARRGET, {"arrget", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {Opcode::ARRSET, {"arrset", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {Opcode::ARRGETLEN, {"arrgetlen", OK_REGISTER, OK_REGISTER}},
  {Opcode::DICTGET, {"dictget", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {Opcode::DICTSET, {"dictset", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {Opcode::DICTGETLEN, {"dictgetlen", OK_REGISTER, OK_REGISTER}},
  {Opcode::NEWINSTANCE, {"newinstance", OK_REGISTER, OK_REGISTER}},
  {Opcode::GETSUPER, {"getsuper", OK_REGISTER, OK_REGISTER}},
  {Opcode::GETSTATIC, {"getstatic", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {Opcode::GETDYNAMIC, {"getdynamic", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {Opcode::SETSTATIC, {"setstatic", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {Opcode::SETDYNAMIC, {"setdynamic", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {Opcode::CALLSTATIC, {"callstatic", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {Opcode::PCALLSTATIC, {"pcallstatic", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {Opcode::CALLDYNAMIC, {"calldynamic", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {Opcode::PCALLDYNAMIC, {"pcalldynamic", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
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
    const Operand* ok = &(lp->il.a) + i++;

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

#undef OK_NONE
#undef OK_GENERIC
#undef OK_REGISTER
#undef OK_CONSTANT
#undef OK_LABEL

} // namespace vm

} // namespace core

} // namespace via
