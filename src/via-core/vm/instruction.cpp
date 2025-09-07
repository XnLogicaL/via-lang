/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "instruction.h"
#include <cstring>
#include <iomanip>
#include "color.h"

using OpCode = via::OpCode;

enum class Operand : via::u8
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
  OpCode opc;
  OperandLayout ol;
};

// TODO
static constexpr InstructionLayout kInstrLayoutMap[] = {
  {"nop", OpCode::NOP, {}},
  {"halt", OpCode::HALT, {}},
  {"extraarg1", OpCode::EXTRAARG1, {Operand::Generic}},
  {"extraarg2", OpCode::EXTRAARG2, {Operand::Generic, Operand::Generic}},
  {"extraarg3",
   OpCode::EXTRAARG3,
   {Operand::Generic, Operand::Generic, Operand::Generic}},
  {"move", OpCode::MOVE, {Operand::Register, Operand::Register}},
  {"xchg", OpCode::XCHG, {Operand::Register, Operand::Register}},
  {"copy", OpCode::COPY, {Operand::Register, Operand::Register}},
  {"loadk", OpCode::LOADK, {Operand::Register, Operand::Generic}},
  {"loadtrue", OpCode::LOADTRUE, {Operand::Register}},
  {"loadfalse", OpCode::LOADFALSE, {Operand::Register}},
  {"newstr", OpCode::NEWSTR, {Operand::Register}},
  {"newarr", OpCode::NEWARR, {Operand::Register}},
  {"newdict", OpCode::NEWDICT, {Operand::Register}},
  {"newtuple", OpCode::NEWTUPLE, {Operand::Register}},
  {"newclosure", OpCode::NEWCLOSURE, {Operand::Register}},
  {"iadd1", OpCode::IADD1, {Operand::Register, Operand::Register}},
  {"iadd2",
   OpCode::IADD2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"iadd1k", OpCode::IADD1K, {Operand::Register, Operand::Constant}},
  {"iadd2k",
   OpCode::IADD2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"fadd1", OpCode::FADD1, {Operand::Register, Operand::Register}},
  {"fadd2",
   OpCode::FADD2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"fadd1k", OpCode::FADD1K, {Operand::Register, Operand::Constant}},
  {"fadd2k",
   OpCode::FADD2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"isub1", OpCode::ISUB1, {Operand::Register, Operand::Register}},
  {"isub2",
   OpCode::ISUB2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"isub1k", OpCode::ISUB1K, {Operand::Register, Operand::Constant}},
  {"isub2k",
   OpCode::ISUB2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"isub2kx",
   OpCode::ISUB2KX,
   {Operand::Register, Operand::Constant, Operand::Register}},
  {"fsub1", OpCode::FSUB1, {Operand::Register, Operand::Register}},
  {"fsub2",
   OpCode::FSUB2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"fsub1k", OpCode::FSUB1K, {Operand::Register, Operand::Constant}},
  {"fsub2k",
   OpCode::FSUB2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"fsub2kx",
   OpCode::FSUB2KX,
   {Operand::Register, Operand::Constant, Operand::Register}},
  {"imul1", OpCode::IMUL1, {Operand::Register, Operand::Register}},
  {"imul2",
   OpCode::IMUL2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"imul1k", OpCode::IMUL1K, {Operand::Register, Operand::Constant}},
  {"imul2k",
   OpCode::IMUL2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"fmul1", OpCode::FMUL1, {Operand::Register, Operand::Register}},
  {"fmul2",
   OpCode::FMUL2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"fmul1k", OpCode::FMUL1K, {Operand::Register, Operand::Constant}},
  {"fmul2k",
   OpCode::FMUL2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"idiv1", OpCode::IDIV1, {Operand::Register, Operand::Register}},
  {"idiv2",
   OpCode::IDIV2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"idiv1k", OpCode::IDIV1K, {Operand::Register, Operand::Constant}},
  {"idiv2k",
   OpCode::IDIV2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"idiv2kx",
   OpCode::IDIV2KX,
   {Operand::Register, Operand::Constant, Operand::Register}},
  {"fdiv1", OpCode::FDIV1, {Operand::Register, Operand::Register}},
  {"fdiv2",
   OpCode::FDIV2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"fdiv1k", OpCode::FDIV1K, {Operand::Register, Operand::Constant}},
  {"fdiv2k",
   OpCode::FDIV2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"fdiv2kx",
   OpCode::FDIV2KX,
   {Operand::Register, Operand::Constant, Operand::Register}},
  {"ipow1", OpCode::IPOW1, {Operand::Register, Operand::Register}},
  {"ipow2",
   OpCode::IPOW2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"ipow1k", OpCode::IPOW1K, {Operand::Register, Operand::Constant}},
  {"ipow2k",
   OpCode::IPOW2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"ipow2kx",
   OpCode::IPOW2KX,
   {Operand::Register, Operand::Constant, Operand::Register}},
  {"fpow1", OpCode::FPOW1, {Operand::Register, Operand::Register}},
  {"fpow2",
   OpCode::FPOW2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"fpow1k", OpCode::FPOW1K, {Operand::Register, Operand::Constant}},
  {"fpow2k",
   OpCode::FPOW2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"fpow2kx",
   OpCode::FPOW2KX,
   {Operand::Register, Operand::Constant, Operand::Register}},
  {"imod1", OpCode::IMOD1, {Operand::Register, Operand::Register}},
  {"imod2",
   OpCode::IMOD2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"imod1k", OpCode::IMOD1K, {Operand::Register, Operand::Constant}},
  {"imod2k",
   OpCode::IMOD2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"imod2kx",
   OpCode::IMOD2KX,
   {Operand::Register, Operand::Constant, Operand::Register}},
  {"fmod1", OpCode::FMOD1, {Operand::Register, Operand::Register}},
  {"fmod2",
   OpCode::FMOD2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"fmod1k", OpCode::FMOD1K, {Operand::Register, Operand::Constant}},
  {"fmod2k",
   OpCode::FMOD2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"fmod2kx",
   OpCode::FMOD2KX,
   {Operand::Register, Operand::Constant, Operand::Register}},
  {"band1", OpCode::BAND1, {Operand::Register, Operand::Register}},
  {"band2",
   OpCode::BAND2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"band1k", OpCode::BAND1K, {Operand::Register, Operand::Constant}},
  {"band2k",
   OpCode::BAND2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"bor1", OpCode::BOR1, {Operand::Register, Operand::Register}},
  {"bor2",
   OpCode::BOR2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"bor1k", OpCode::BOR1K, {Operand::Register, Operand::Constant}},
  {"bor2k",
   OpCode::BOR2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"bxor1", OpCode::BXOR1, {Operand::Register, Operand::Register}},
  {"bxor2",
   OpCode::BXOR2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"bxor1k", OpCode::BXOR1K, {Operand::Register, Operand::Constant}},
  {"bxor2k",
   OpCode::BXOR2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"bshl1", OpCode::BSHL1, {Operand::Register, Operand::Register}},
  {"bshl2",
   OpCode::BSHL2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"bshl1k", OpCode::BSHL1K, {Operand::Register, Operand::Constant}},
  {"bshl2k",
   OpCode::BSHL2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"bshr1", OpCode::BSHR1, {Operand::Register, Operand::Register}},
  {"bshr2",
   OpCode::BSHR2,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"bshr1k", OpCode::BSHR1K, {Operand::Register, Operand::Constant}},
  {"bshr2k",
   OpCode::BSHR2K,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"bnot", OpCode::BNOT, {Operand::Register, Operand::Register}},
  {"bnotk", OpCode::BNOTK, {Operand::Register, Operand::Constant}},
  {"and",
   OpCode::AND,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"andk",
   OpCode::ANDK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"or", OpCode::OR, {Operand::Register, Operand::Register, Operand::Register}},
  {"ork",
   OpCode::ORK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"ieq",
   OpCode::IEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"ieqk",
   OpCode::IEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"feq",
   OpCode::FEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"feqk",
   OpCode::FEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"beq",
   OpCode::BEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"beqk",
   OpCode::BEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"seq",
   OpCode::SEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"seqk",
   OpCode::SEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"ineq",
   OpCode::INEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"ineqk",
   OpCode::INEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"fneq",
   OpCode::FNEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"fneqk",
   OpCode::FNEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"bneq",
   OpCode::BNEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"bneqk",
   OpCode::BNEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"sneq",
   OpCode::SNEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"sneqk",
   OpCode::SNEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"is", OpCode::IS, {Operand::Register, Operand::Register, Operand::Register}},
  {"ilt",
   OpCode::ILT,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"iltk",
   OpCode::ILTK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"flt",
   OpCode::FLT,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"fltk",
   OpCode::FLTK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"igt",
   OpCode::IGT,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"igtk",
   OpCode::IGTK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"fgt",
   OpCode::FGT,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"fgtk",
   OpCode::FGTK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"ilteq",
   OpCode::ILTEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"ilteqk",
   OpCode::ILTEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"flteq",
   OpCode::FLTEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"flteqk",
   OpCode::FLTEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"igteq",
   OpCode::IGTEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"igteqk",
   OpCode::IGTEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"fgteq",
   OpCode::FGTEQ,
   {Operand::Register, Operand::Register, Operand::Register}},
  {"fgteqk",
   OpCode::FGTEQK,
   {Operand::Register, Operand::Register, Operand::Constant}},
  {"not", OpCode::NOT, {Operand::Register, Operand::Register}},
  {"jmp", OpCode::JMP, {Operand::Label}},
  {"jmpif", OpCode::JMPIF, {Operand::Register, Operand::Label}},
  {"savesp", OpCode::SAVESP, {}},
  {"restsp", OpCode::RESTSP, {}},
  {"push", OpCode::PUSH, {Operand::Register}},
  {"pushk", OpCode::PUSHK, {Operand::Constant}},
  {"getarg", OpCode::GETARG, {Operand::Register, Operand::Generic}},
  {"getargref", OpCode::GETARGREF, {Operand::Register, Operand::Generic}},
  {"setarg", OpCode::SETARG, {Operand::Register, Operand::Generic}},
  {"getlocal", OpCode::GETLOCAL, {Operand::Register, Operand::Generic}},
  {"getlocalref", OpCode::GETLOCALREF, {Operand::Register, Operand::Generic}},
  {"setlocal", OpCode::SETLOCAL, {Operand::Register, Operand::Generic}},
  {"btoi", OpCode::BTOI, {Operand::Register, Operand::Register}},
  {"ftoi", OpCode::FTOI, {Operand::Register, Operand::Register}},
  {"stoi", OpCode::STOI, {Operand::Register, Operand::Register}},
  {"itof", OpCode::ITOF, {Operand::Register, Operand::Register}},
  {"btof", OpCode::BTOF, {Operand::Register, Operand::Register}},
  {"stof", OpCode::STOF, {Operand::Register, Operand::Register}},
  {"itob", OpCode::ITOB, {Operand::Register, Operand::Register}},
  {"stob", OpCode::STOB, {Operand::Register, Operand::Register}},
  {"itos", OpCode::ITOS, {Operand::Register, Operand::Register}},
  {"ftos", OpCode::FTOS, {Operand::Register, Operand::Register}},
  {"btos", OpCode::BTOS, {Operand::Register, Operand::Register}},
  {"artos", OpCode::ARTOS, {Operand::Register, Operand::Register}},
  {"dttos", OpCode::DTTOS, {Operand::Register, Operand::Register}},
  {"fntos", OpCode::FNTOS, {Operand::Register, Operand::Register}},
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

std::string via::Instruction::dump() const
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
