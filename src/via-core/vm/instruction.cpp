// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "instruction.h"
#include <cstring>
#include <iomanip>
#include "color.h"

namespace via
{

enum class Operand : u8
{
  None,
  Generic,
  Register,
  Constant,
  Label,
};

struct OperandLayout
{
  Operand a = Operand::None, b = Operand::None, c = Operand::None;
};

struct InstructionLayout
{
  const char* op_str;
  Opcode opc;
  OperandLayout ol;
};

// TODO
static constexpr InstructionLayout kInstrLayoutMap[] = {
    {"nop", Opcode::NOP, {}},
    {"halt", Opcode::HALT, {}},
    {"extraarg1", Opcode::EXTRAARG1, {Operand::Generic}},
    {"extraarg2", Opcode::EXTRAARG2, {Operand::Generic, Operand::Generic}},
    {"extraarg3",
     Opcode::EXTRAARG3,
     {Operand::Generic, Operand::Generic, Operand::Generic}},
    {"move", Opcode::MOVE, {Operand::Register, Operand::Register}},
    {"xchg", Opcode::XCHG, {Operand::Register, Operand::Register}},
    {"copy", Opcode::COPY, {Operand::Register, Operand::Register}},
    {"loadk", Opcode::LOADK, {Operand::Register, Operand::Generic}},
    {"loadtrue", Opcode::LOADTRUE, {Operand::Register}},
    {"loadfalse", Opcode::LOADFALSE, {Operand::Register}},
    {"newstr", Opcode::NEWSTR, {Operand::Register}},
    {"newarr", Opcode::NEWARR, {Operand::Register}},
    {"newdict", Opcode::NEWDICT, {Operand::Register}},
    {"newtuple", Opcode::NEWTUPLE, {Operand::Register}},
    {"newclosure", Opcode::NEWCLOSURE, {Operand::Register}},
    {"iadd1", Opcode::IADD1, {Operand::Register, Operand::Register}},
    {"iadd2",
     Opcode::IADD2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"iadd1k", Opcode::IADD1K, {Operand::Register, Operand::Constant}},
    {"iadd2k",
     Opcode::IADD2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"fadd1", Opcode::FADD1, {Operand::Register, Operand::Register}},
    {"fadd2",
     Opcode::FADD2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"fadd1k", Opcode::FADD1K, {Operand::Register, Operand::Constant}},
    {"fadd2k",
     Opcode::FADD2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"isub1", Opcode::ISUB1, {Operand::Register, Operand::Register}},
    {"isub2",
     Opcode::ISUB2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"isub1k", Opcode::ISUB1K, {Operand::Register, Operand::Constant}},
    {"isub2k",
     Opcode::ISUB2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"isub2kx",
     Opcode::ISUB2KX,
     {Operand::Register, Operand::Constant, Operand::Register}},
    {"fsub1", Opcode::FSUB1, {Operand::Register, Operand::Register}},
    {"fsub2",
     Opcode::FSUB2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"fsub1k", Opcode::FSUB1K, {Operand::Register, Operand::Constant}},
    {"fsub2k",
     Opcode::FSUB2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"fsub2kx",
     Opcode::FSUB2KX,
     {Operand::Register, Operand::Constant, Operand::Register}},
    {"imul1", Opcode::IMUL1, {Operand::Register, Operand::Register}},
    {"imul2",
     Opcode::IMUL2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"imul1k", Opcode::IMUL1K, {Operand::Register, Operand::Constant}},
    {"imul2k",
     Opcode::IMUL2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"fmul1", Opcode::FMUL1, {Operand::Register, Operand::Register}},
    {"fmul2",
     Opcode::FMUL2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"fmul1k", Opcode::FMUL1K, {Operand::Register, Operand::Constant}},
    {"fmul2k",
     Opcode::FMUL2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"idiv1", Opcode::IDIV1, {Operand::Register, Operand::Register}},
    {"idiv2",
     Opcode::IDIV2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"idiv1k", Opcode::IDIV1K, {Operand::Register, Operand::Constant}},
    {"idiv2k",
     Opcode::IDIV2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"idiv2kx",
     Opcode::IDIV2KX,
     {Operand::Register, Operand::Constant, Operand::Register}},
    {"fdiv1", Opcode::FDIV1, {Operand::Register, Operand::Register}},
    {"fdiv2",
     Opcode::FDIV2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"fdiv1k", Opcode::FDIV1K, {Operand::Register, Operand::Constant}},
    {"fdiv2k",
     Opcode::FDIV2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"fdiv2kx",
     Opcode::FDIV2KX,
     {Operand::Register, Operand::Constant, Operand::Register}},
    {"ipow1", Opcode::IPOW1, {Operand::Register, Operand::Register}},
    {"ipow2",
     Opcode::IPOW2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"ipow1k", Opcode::IPOW1K, {Operand::Register, Operand::Constant}},
    {"ipow2k",
     Opcode::IPOW2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"ipow2kx",
     Opcode::IPOW2KX,
     {Operand::Register, Operand::Constant, Operand::Register}},
    {"fpow1", Opcode::FPOW1, {Operand::Register, Operand::Register}},
    {"fpow2",
     Opcode::FPOW2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"fpow1k", Opcode::FPOW1K, {Operand::Register, Operand::Constant}},
    {"fpow2k",
     Opcode::FPOW2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"fpow2kx",
     Opcode::FPOW2KX,
     {Operand::Register, Operand::Constant, Operand::Register}},
    {"imod1", Opcode::IMOD1, {Operand::Register, Operand::Register}},
    {"imod2",
     Opcode::IMOD2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"imod1k", Opcode::IMOD1K, {Operand::Register, Operand::Constant}},
    {"imod2k",
     Opcode::IMOD2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"imod2kx",
     Opcode::IMOD2KX,
     {Operand::Register, Operand::Constant, Operand::Register}},
    {"fmod1", Opcode::FMOD1, {Operand::Register, Operand::Register}},
    {"fmod2",
     Opcode::FMOD2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"fmod1k", Opcode::FMOD1K, {Operand::Register, Operand::Constant}},
    {"fmod2k",
     Opcode::FMOD2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"fmod2kx",
     Opcode::FMOD2KX,
     {Operand::Register, Operand::Constant, Operand::Register}},
    {"band1", Opcode::BAND1, {Operand::Register, Operand::Register}},
    {"band2",
     Opcode::BAND2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"band1k", Opcode::BAND1K, {Operand::Register, Operand::Constant}},
    {"band2k",
     Opcode::BAND2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"bor1", Opcode::BOR1, {Operand::Register, Operand::Register}},
    {"bor2",
     Opcode::BOR2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"bor1k", Opcode::BOR1K, {Operand::Register, Operand::Constant}},
    {"bor2k",
     Opcode::BOR2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"bxor1", Opcode::BXOR1, {Operand::Register, Operand::Register}},
    {"bxor2",
     Opcode::BXOR2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"bxor1k", Opcode::BXOR1K, {Operand::Register, Operand::Constant}},
    {"bxor2k",
     Opcode::BXOR2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"bshl1", Opcode::BSHL1, {Operand::Register, Operand::Register}},
    {"bshl2",
     Opcode::BSHL2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"bshl1k", Opcode::BSHL1K, {Operand::Register, Operand::Constant}},
    {"bshl2k",
     Opcode::BSHL2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"bshr1", Opcode::BSHR1, {Operand::Register, Operand::Register}},
    {"bshr2",
     Opcode::BSHR2,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"bshr1k", Opcode::BSHR1K, {Operand::Register, Operand::Constant}},
    {"bshr2k",
     Opcode::BSHR2K,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"bnot", Opcode::BNOT, {Operand::Register, Operand::Register}},
    {"bnotk", Opcode::BNOTK, {Operand::Register, Operand::Constant}},
    {"and",
     Opcode::AND,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"andk",
     Opcode::ANDK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"or",
     Opcode::OR,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"ork",
     Opcode::ORK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"ieq",
     Opcode::IEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"ieqk",
     Opcode::IEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"feq",
     Opcode::FEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"feqk",
     Opcode::FEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"beq",
     Opcode::BEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"beqk",
     Opcode::BEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"seq",
     Opcode::SEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"seqk",
     Opcode::SEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"ineq",
     Opcode::INEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"ineqk",
     Opcode::INEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"fneq",
     Opcode::FNEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"fneqk",
     Opcode::FNEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"bneq",
     Opcode::BNEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"bneqk",
     Opcode::BNEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"sneq",
     Opcode::SNEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"sneqk",
     Opcode::SNEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"is",
     Opcode::IS,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"ilt",
     Opcode::ILT,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"iltk",
     Opcode::ILTK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"flt",
     Opcode::FLT,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"fltk",
     Opcode::FLTK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"igt",
     Opcode::IGT,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"igtk",
     Opcode::IGTK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"fgt",
     Opcode::FGT,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"fgtk",
     Opcode::FGTK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"ilteq",
     Opcode::ILTEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"ilteqk",
     Opcode::ILTEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"flteq",
     Opcode::FLTEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"flteqk",
     Opcode::FLTEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"igteq",
     Opcode::IGTEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"igteqk",
     Opcode::IGTEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"fgteq",
     Opcode::FGTEQ,
     {Operand::Register, Operand::Register, Operand::Register}},
    {"fgteqk",
     Opcode::FGTEQK,
     {Operand::Register, Operand::Register, Operand::Constant}},
    {"not", Opcode::NOT, {Operand::Register, Operand::Register}},
    {"jmp", Opcode::JMP, {Operand::Label}},
    {"jmpif", Opcode::JMPIF, {Operand::Register, Operand::Label}},
    {"savesp", Opcode::SAVESP, {}},
    {"restsp", Opcode::RESTSP, {}},
    {"push", Opcode::PUSH, {Operand::Register}},
    {"pushk", Opcode::PUSHK, {Operand::Constant}},
    {"getarg", Opcode::GETARG, {Operand::Register, Operand::Generic}},
    {"getargref", Opcode::GETARGREF, {Operand::Register, Operand::Generic}},
    {"setarg", Opcode::SETARG, {Operand::Register, Operand::Generic}},
    {"getlocal", Opcode::GETLOCAL, {Operand::Register, Operand::Generic}},
    {"getlocalref", Opcode::GETLOCALREF, {Operand::Register, Operand::Generic}},
    {"setlocal", Opcode::SETLOCAL, {Operand::Register, Operand::Generic}},
    {"btoi", Opcode::BTOI, {Operand::Register, Operand::Register}},
    {"ftoi", Opcode::FTOI, {Operand::Register, Operand::Register}},
    {"stoi", Opcode::STOI, {Operand::Register, Operand::Register}},
    {"itof", Opcode::ITOF, {Operand::Register, Operand::Register}},
    {"btof", Opcode::BTOF, {Operand::Register, Operand::Register}},
    {"stof", Opcode::STOF, {Operand::Register, Operand::Register}},
    {"itob", Opcode::ITOB, {Operand::Register, Operand::Register}},
    {"stob", Opcode::STOB, {Operand::Register, Operand::Register}},
    {"itos", Opcode::ITOS, {Operand::Register, Operand::Register}},
    {"ftos", Opcode::FTOS, {Operand::Register, Operand::Register}},
    {"btos", Opcode::BTOS, {Operand::Register, Operand::Register}},
    {"artos", Opcode::ARTOS, {Operand::Register, Operand::Register}},
    {"dttos", Opcode::DTTOS, {Operand::Register, Operand::Register}},
    {"fntos", Opcode::FNTOS, {Operand::Register, Operand::Register}},
    // ...
};

static char getOperandPrefix(Operand kind)
{
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

std::string Instruction::dump() const
{
  const u16 ops[] = {a, b, c};
  const InstructionLayout* il = nullptr;

  for (const auto& pair : kInstrLayoutMap) {
    if (pair.opc == op) {
      il = &pair;
      break;
    }
  }

  if (il == nullptr) {
    return "<unmapped-instruction>";
  }

  std::ostringstream oss;
  oss << std::left << std::setfill(' ') << std::setw(20);
  oss << ansiFormat(il->op_str, Fg::Magenta, Bg::Black, Style::Bold) << " ";

  for (size_t i = 0; u16 operand : ops) {
    // disgusting addressing hack
    const Operand* ok;
    if ((ok = &(il->ol.a) + i++, *ok == Operand::None)) {
      break;
    }

    std::string realPrefix;

    char pref;
    if ((pref = getOperandPrefix(*ok), pref == 0))
      realPrefix = "";
    else
      realPrefix = pref;

    oss << realPrefix << std::to_string(operand);

    // another address hack...
    // check if next operand is used and append seperator accordingly
    if (*(ok + 1) != Operand::None) {
      oss << " ";
    }
  }

  return oss.str();
}

}  // namespace via
