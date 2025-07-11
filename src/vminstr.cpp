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
  {VOP_EXTRAARG1, {"extraarg1", OK_GENERIC}},
  {VOP_EXTRAARG2, {"extraarg2", OK_GENERIC, OK_GENERIC}},
  {VOP_EXTRAARG3, {"extraarg3", OK_GENERIC, OK_GENERIC, OK_GENERIC}},
  {VOP_IADD1, {"iadd1", OK_REGISTER, OK_REGISTER}},
  {VOP_IADD2, {"iadd2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_IADD1K, {"iadd1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_IADD2K, {"iadd2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FADD1, {"fadd1", OK_REGISTER, OK_REGISTER}},
  {VOP_FADD2, {"fadd2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FADD1K, {"fadd1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_FADD2K, {"fadd2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FADD1X, {"fadd1x", OK_REGISTER, OK_REGISTER}},
  {VOP_FADD2X, {"fadd2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FADD1XK, {"fadd1xk", OK_REGISTER, OK_CONSTANT}},
  {VOP_FADD2XK, {"fadd2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_ISUB1, {"isub1", OK_REGISTER, OK_REGISTER}},
  {VOP_ISUB2, {"isub2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_ISUB1K, {"isub1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_ISUB2K, {"isub2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FSUB1, {"fsub1", OK_REGISTER, OK_REGISTER}},
  {VOP_FSUB2, {"fsub2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FSUB1K, {"fsub1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_FSUB2K, {"fsub2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FSUB1X, {"fsub1x", OK_REGISTER, OK_REGISTER}},
  {VOP_FSUB2X, {"fsub2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FSUB1XK, {"fsub1xk", OK_REGISTER, OK_CONSTANT}},
  {VOP_FSUB2XK, {"fsub2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_IMUL1, {"imul1", OK_REGISTER, OK_REGISTER}},
  {VOP_IMUL2, {"imul2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_IMUL1K, {"imul1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_IMUL2K, {"imul2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FMUL1, {"fmul1", OK_REGISTER, OK_REGISTER}},
  {VOP_FMUL2, {"fmul2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FMUL1K, {"fmul1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_FMUL2K, {"fmul2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FMUL1X, {"fmul1x", OK_REGISTER, OK_REGISTER}},
  {VOP_FMUL2X, {"fmul2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FMUL1XK, {"fmul1xk", OK_REGISTER, OK_CONSTANT}},
  {VOP_FMUL2XK, {"fmul2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_IDIV1, {"idiv1", OK_REGISTER, OK_REGISTER}},
  {VOP_IDIV2, {"idiv2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_IDIV1K, {"idiv1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_IDIV2K, {"idiv2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FDIV1, {"fdiv1", OK_REGISTER, OK_REGISTER}},
  {VOP_FDIV2, {"fdiv2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FDIV1K, {"fdiv1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_FDIV2K, {"fdiv2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FDIV1X, {"fdiv1x", OK_REGISTER, OK_REGISTER}},
  {VOP_FDIV2X, {"fdiv2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FDIV1XY, {"fdiv1xy", OK_REGISTER, OK_REGISTER}},
  {VOP_FDIV2XY, {"fdiv2xy", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FDIV1XK, {"fdiv1xk", OK_REGISTER, OK_CONSTANT}},
  {VOP_FDIV2XK, {"fdiv2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FDIV1XYK, {"fdiv1xyk", OK_REGISTER, OK_CONSTANT}},
  {VOP_FDIV2XYK, {"fdiv2xyk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_IPOW1, {"ipow1", OK_REGISTER, OK_REGISTER}},
  {VOP_IPOW2, {"ipow2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_IPOW1K, {"ipow1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_IPOW2K, {"ipow2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FPOW1, {"fpow1", OK_REGISTER, OK_REGISTER}},
  {VOP_FPOW2, {"fpow2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FPOW1K, {"fpow1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_FPOW2K, {"fpow2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FPOW1X, {"fpow1x", OK_REGISTER, OK_REGISTER}},
  {VOP_FPOW2X, {"fpow2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FPOW1XY, {"fpow1xy", OK_REGISTER, OK_REGISTER}},
  {VOP_FPOW2XY, {"fpow2xy", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FPOW1XK, {"fpow1xk", OK_REGISTER, OK_CONSTANT}},
  {VOP_FPOW2XK, {"fpow2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FPOW1XYK, {"fpow1xyk", OK_REGISTER, OK_CONSTANT}},
  {VOP_FPOW2XYK, {"fpow2xyk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_IMOD1, {"imod1", OK_REGISTER, OK_REGISTER}},
  {VOP_IMOD2, {"imod2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_IMOD1K, {"imod1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_IMOD2K, {"imod2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FMOD1, {"fmod1", OK_REGISTER, OK_REGISTER}},
  {VOP_FMOD2, {"fmod2", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FMOD1K, {"fmod1k", OK_REGISTER, OK_CONSTANT}},
  {VOP_FMOD2K, {"fmod2k", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FMOD1X, {"fmod1x", OK_REGISTER, OK_REGISTER}},
  {VOP_FMOD2X, {"fmod2x", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FMOD1XY, {"fmod1xy", OK_REGISTER, OK_REGISTER}},
  {VOP_FMOD2XY, {"fmod2xy", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_FMOD1XK, {"fmod1xk", OK_REGISTER, OK_CONSTANT}},
  {VOP_FMOD2XK, {"fmod2xk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_FMOD1XYK, {"fmod1xyk", OK_REGISTER, OK_CONSTANT}},
  {VOP_FMOD2XYK, {"fmod2xyk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_MOVE, {"move", OK_REGISTER, OK_REGISTER}},
  {VOP_XCHG, {"xchg", OK_REGISTER, OK_REGISTER}},
  {VOP_COPY, {"copy", OK_REGISTER, OK_REGISTER}},
  {VOP_COPYREF, {"copyref", OK_REGISTER, OK_REGISTER}},
  {VOP_LOADTRUE, {"loadtrue", OK_REGISTER}},
  {VOP_LOADFALSE, {"loadfalse", OK_REGISTER}},
  {VOP_NEWSTR, {"newstr", OK_REGISTER}},
  {VOP_NEWSTR2, {"newstr2", OK_REGISTER, OK_GENERIC}},
  {VOP_NEWARR, {"newarr", OK_REGISTER}},
  {VOP_NEWARR2, {"newarr2", OK_REGISTER, OK_GENERIC}},
  {VOP_NEWDICT, {"newdict", OK_REGISTER}},
  {VOP_NEWTUPLE, {"newtuple", OK_REGISTER, OK_GENERIC}},
  {VOP_NEWCLOSURE, {"newclosure", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSH, {"push", OK_REGISTER}},
  {VOP_PUSHK, {"pushk", OK_CONSTANT}},
  {VOP_GETARG, {"getarg", OK_REGISTER, OK_GENERIC}},
  {VOP_GETARGREF, {"getargref", OK_REGISTER, OK_GENERIC}},
  {VOP_SETARG, {"setarg", OK_REGISTER, OK_GENERIC}},
  {VOP_GETLOCAL, {"getlocal", OK_REGISTER, OK_GENERIC}},
  {VOP_GETLOCALREF, {"getlocalref", OK_REGISTER, OK_GENERIC}},
  {VOP_SETLOCAL, {"setlocal", OK_REGISTER, OK_GENERIC}},
  {VOP_DUPLOCAL, {"duplocal", OK_GENERIC}},
  {VOP_DUPLOCALREF, {"duplocalref", OK_GENERIC}},
  {VOP_PUSHIADD, {"pushiadd", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHIADDK, {"pushiaddk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFADD, {"pushfadd", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFADDK, {"pushfaddk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFADDX, {"pushfaddx", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFADDXK, {"pushfaddxk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHISUB, {"pushisub", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHISUBK, {"pushisubk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFSUB, {"pushfsub", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFSUBK, {"pushfsubk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFSUBX, {"pushfsubx", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFSUBXK, {"pushfsubxk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHISUB, {"pushimul", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHISUBK, {"pushimulk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFSUB, {"pushfmul", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFSUBK, {"pushfmulk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFSUBX, {"pushfmulx", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFSUBXK, {"pushfmulxk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHIDIV, {"pushidiv", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHIDIVK, {"pushidivk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFDIV, {"pushfdiv", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFDIVK, {"pushfdivk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFDIVX, {"pushfdivx", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFDIVXY, {"pushfdivxy", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFDIVXK, {"pushfdivxk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFDIVXYK, {"pushfdivxyk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHIPOW, {"pushipow", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHIPOWK, {"pushipowk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFPOW, {"pushfpow", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFPOWK, {"pushfpowk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFPOWX, {"pushfpowx", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFPOWXY, {"pushfpowxy", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFPOWXK, {"pushfpowxk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFPOWXYK, {"pushfpowxyk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHIMOD, {"pushimod", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHIMODK, {"pushimodk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFMOD, {"pushfmod", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFMODK, {"pushfmodk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFMODX, {"pushfmodx", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFMODXY, {"pushfmodxy", OK_REGISTER, OK_REGISTER}},
  {VOP_PUSHFMODXK, {"pushfmodxk", OK_REGISTER, OK_CONSTANT}},
  {VOP_PUSHFMODXYK, {"pushfmodxyk", OK_REGISTER, OK_CONSTANT}},
  {VOP_ICASTB, {"icastb", OK_REGISTER, OK_REGISTER}},
  {VOP_ICASTF, {"icastf", OK_REGISTER, OK_REGISTER}},
  {VOP_ICASTSTR, {"icaststr", OK_REGISTER, OK_REGISTER}},
  {VOP_FCASTI, {"fcasti", OK_REGISTER, OK_REGISTER}},
  {VOP_FCASTB, {"fcastb", OK_REGISTER, OK_REGISTER}},
  {VOP_FCASTSTR, {"fcaststr", OK_REGISTER, OK_REGISTER}},
  {VOP_BCASTI, {"bcasti", OK_REGISTER, OK_REGISTER}},
  {VOP_BCASTSTR, {"bcaststr", OK_REGISTER, OK_REGISTER}},
  {VOP_STRCASTI, {"strcasti", OK_REGISTER, OK_REGISTER}},
  {VOP_STRCASTF, {"strcastf", OK_REGISTER, OK_REGISTER}},
  {VOP_STRCASTB, {"strcastb", OK_REGISTER, OK_REGISTER}},
  {VOP_STRCASTARR, {"strcastarr", OK_REGISTER, OK_REGISTER}},
  {VOP_STRCASTDICT, {"strcastdict", OK_REGISTER, OK_REGISTER}},
  {VOP_STRCASTFUNC, {"strcastfunc", OK_REGISTER, OK_REGISTER}},
  {VOP_CAPTURE, {"capture", OK_GENERIC}},
  {VOP_CALL, {"call", OK_REGISTER, OK_GENERIC}},
  {VOP_PCALL, {"pcall", OK_REGISTER, OK_GENERIC}},
  {VOP_RET, {"ret", OK_REGISTER}},
  {VOP_RETNIL, {"retnil"}},
  {VOP_RETTRUE, {"rettrue"}},
  {VOP_RETFALSE, {"retfalse"}},
  {VOP_RETK, {"retk", OK_CONSTANT}},
  {VOP_GETUPV, {"getupv", OK_REGISTER, OK_GENERIC}},
  {VOP_GETUPVREF, {"getupvref", OK_REGISTER, OK_GENERIC}},
  {VOP_SETUPV, {"setupv", OK_REGISTER, OK_GENERIC}},
  {VOP_STRGET, {"strget", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {VOP_STRSET, {"strset", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {VOP_STRGETLEN, {"strgetlen", OK_REGISTER, OK_REGISTER}},
  {VOP_STRCONCAT, {"strconcat", OK_REGISTER, OK_REGISTER, OK_REGISTER}},
  {VOP_STRCONCATK, {"strconcatk", OK_REGISTER, OK_REGISTER, OK_CONSTANT}},
  {VOP_ARRGET, {"arrget", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {VOP_ARRSET, {"arrset", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {VOP_ARRGETLEN, {"arrgetlen", OK_REGISTER, OK_REGISTER}},
  {VOP_DICTGET, {"dictget", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {VOP_DICTSET, {"dictset", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {VOP_DICTGETLEN, {"dictgetlen", OK_REGISTER, OK_REGISTER}},
  {VOP_NEWINSTANCE, {"newinstance", OK_REGISTER, OK_REGISTER}},
  {VOP_GETSUPER, {"getsuper", OK_REGISTER, OK_REGISTER}},
  {VOP_GETSTATIC, {"getstatic", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {VOP_GETDYNAMIC, {"getdynamic", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {VOP_SETSTATIC, {"setstatic", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {VOP_SETDYNAMIC, {"setdynamic", OK_REGISTER, OK_REGISTER, OK_GENERIC}},
  {VOP_CALLSTATIC, {"callstatic", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {VOP_PCALLSTATIC, {"pcallstatic", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {VOP_CALLDYNAMIC, {"calldynamic", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
  {VOP_PCALLDYNAMIC, {"pcalldynamic", OK_REGISTER, OK_GENERIC, OK_GENERIC}},
};

Opcode opcode_from_string(const char* str) {
  for (const auto& pair : insn_layout_map)
    if (strcmp(str, pair.il.opc) == 0)
      return pair.opc;

  return VOP_NOP;
}

const char* opcode_to_string(Opcode opc) {
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
