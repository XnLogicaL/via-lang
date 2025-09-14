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
#include "ansi.h"

using OpCode = via::OpCode;

enum Operand : via::u8
{
  NONE,
  GENERIC,
  REGISTER,
  CONSTANT,
  LABEL,
};

struct OperandLayout
{
  Operand a = NONE, b = NONE, c = NONE;
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
  {"extraarg1", OpCode::EXTRAARG1, {GENERIC}},
  {"extraarg2", OpCode::EXTRAARG2, {GENERIC, GENERIC}},
  {"extraarg3", OpCode::EXTRAARG3, {GENERIC, GENERIC, GENERIC}},
  {"move", OpCode::MOVE, {REGISTER, REGISTER}},
  {"xchg", OpCode::XCHG, {REGISTER, REGISTER}},
  {"copy", OpCode::COPY, {REGISTER, REGISTER}},
  {"loadk", OpCode::LOADK, {REGISTER, GENERIC}},
  {"loadtrue", OpCode::LOADTRUE, {REGISTER}},
  {"loadfalse", OpCode::LOADFALSE, {REGISTER}},
  {"newstr", OpCode::NEWSTR, {REGISTER}},
  {"newarr", OpCode::NEWARR, {REGISTER}},
  {"newdict", OpCode::NEWDICT, {REGISTER}},
  {"newtuple", OpCode::NEWTUPLE, {REGISTER}},
  {"newclosure", OpCode::NEWCLOSURE, {REGISTER}},
  {"iadd1", OpCode::IADD1, {REGISTER, REGISTER}},
  {"iadd2", OpCode::IADD2, {REGISTER, REGISTER, REGISTER}},
  {"iadd1k", OpCode::IADD1K, {REGISTER, CONSTANT}},
  {"iadd2k", OpCode::IADD2K, {REGISTER, REGISTER, CONSTANT}},
  {"fadd1", OpCode::FADD1, {REGISTER, REGISTER}},
  {"fadd2", OpCode::FADD2, {REGISTER, REGISTER, REGISTER}},
  {"fadd1k", OpCode::FADD1K, {REGISTER, CONSTANT}},
  {"fadd2k", OpCode::FADD2K, {REGISTER, REGISTER, CONSTANT}},
  {"isub1", OpCode::ISUB1, {REGISTER, REGISTER}},
  {"isub2", OpCode::ISUB2, {REGISTER, REGISTER, REGISTER}},
  {"isub1k", OpCode::ISUB1K, {REGISTER, CONSTANT}},
  {"isub2k", OpCode::ISUB2K, {REGISTER, REGISTER, CONSTANT}},
  {"isub2kx", OpCode::ISUB2KX, {REGISTER, CONSTANT, REGISTER}},
  {"fsub1", OpCode::FSUB1, {REGISTER, REGISTER}},
  {"fsub2", OpCode::FSUB2, {REGISTER, REGISTER, REGISTER}},
  {"fsub1k", OpCode::FSUB1K, {REGISTER, CONSTANT}},
  {"fsub2k", OpCode::FSUB2K, {REGISTER, REGISTER, CONSTANT}},
  {"fsub2kx", OpCode::FSUB2KX, {REGISTER, CONSTANT, REGISTER}},
  {"imul1", OpCode::IMUL1, {REGISTER, REGISTER}},
  {"imul2", OpCode::IMUL2, {REGISTER, REGISTER, REGISTER}},
  {"imul1k", OpCode::IMUL1K, {REGISTER, CONSTANT}},
  {"imul2k", OpCode::IMUL2K, {REGISTER, REGISTER, CONSTANT}},
  {"fmul1", OpCode::FMUL1, {REGISTER, REGISTER}},
  {"fmul2", OpCode::FMUL2, {REGISTER, REGISTER, REGISTER}},
  {"fmul1k", OpCode::FMUL1K, {REGISTER, CONSTANT}},
  {"fmul2k", OpCode::FMUL2K, {REGISTER, REGISTER, CONSTANT}},
  {"idiv1", OpCode::IDIV1, {REGISTER, REGISTER}},
  {"idiv2", OpCode::IDIV2, {REGISTER, REGISTER, REGISTER}},
  {"idiv1k", OpCode::IDIV1K, {REGISTER, CONSTANT}},
  {"idiv2k", OpCode::IDIV2K, {REGISTER, REGISTER, CONSTANT}},
  {"idiv2kx", OpCode::IDIV2KX, {REGISTER, CONSTANT, REGISTER}},
  {"fdiv1", OpCode::FDIV1, {REGISTER, REGISTER}},
  {"fdiv2", OpCode::FDIV2, {REGISTER, REGISTER, REGISTER}},
  {"fdiv1k", OpCode::FDIV1K, {REGISTER, CONSTANT}},
  {"fdiv2k", OpCode::FDIV2K, {REGISTER, REGISTER, CONSTANT}},
  {"fdiv2kx", OpCode::FDIV2KX, {REGISTER, CONSTANT, REGISTER}},
  {"ipow1", OpCode::IPOW1, {REGISTER, REGISTER}},
  {"ipow2", OpCode::IPOW2, {REGISTER, REGISTER, REGISTER}},
  {"ipow1k", OpCode::IPOW1K, {REGISTER, CONSTANT}},
  {"ipow2k", OpCode::IPOW2K, {REGISTER, REGISTER, CONSTANT}},
  {"ipow2kx", OpCode::IPOW2KX, {REGISTER, CONSTANT, REGISTER}},
  {"fpow1", OpCode::FPOW1, {REGISTER, REGISTER}},
  {"fpow2", OpCode::FPOW2, {REGISTER, REGISTER, REGISTER}},
  {"fpow1k", OpCode::FPOW1K, {REGISTER, CONSTANT}},
  {"fpow2k", OpCode::FPOW2K, {REGISTER, REGISTER, CONSTANT}},
  {"fpow2kx", OpCode::FPOW2KX, {REGISTER, CONSTANT, REGISTER}},
  {"imod1", OpCode::IMOD1, {REGISTER, REGISTER}},
  {"imod2", OpCode::IMOD2, {REGISTER, REGISTER, REGISTER}},
  {"imod1k", OpCode::IMOD1K, {REGISTER, CONSTANT}},
  {"imod2k", OpCode::IMOD2K, {REGISTER, REGISTER, CONSTANT}},
  {"imod2kx", OpCode::IMOD2KX, {REGISTER, CONSTANT, REGISTER}},
  {"fmod1", OpCode::FMOD1, {REGISTER, REGISTER}},
  {"fmod2", OpCode::FMOD2, {REGISTER, REGISTER, REGISTER}},
  {"fmod1k", OpCode::FMOD1K, {REGISTER, CONSTANT}},
  {"fmod2k", OpCode::FMOD2K, {REGISTER, REGISTER, CONSTANT}},
  {"fmod2kx", OpCode::FMOD2KX, {REGISTER, CONSTANT, REGISTER}},
  {"band1", OpCode::BAND1, {REGISTER, REGISTER}},
  {"band2", OpCode::BAND2, {REGISTER, REGISTER, REGISTER}},
  {"band1k", OpCode::BAND1K, {REGISTER, CONSTANT}},
  {"band2k", OpCode::BAND2K, {REGISTER, REGISTER, CONSTANT}},
  {"bor1", OpCode::BOR1, {REGISTER, REGISTER}},
  {"bor2", OpCode::BOR2, {REGISTER, REGISTER, REGISTER}},
  {"bor1k", OpCode::BOR1K, {REGISTER, CONSTANT}},
  {"bor2k", OpCode::BOR2K, {REGISTER, REGISTER, CONSTANT}},
  {"bxor1", OpCode::BXOR1, {REGISTER, REGISTER}},
  {"bxor2", OpCode::BXOR2, {REGISTER, REGISTER, REGISTER}},
  {"bxor1k", OpCode::BXOR1K, {REGISTER, CONSTANT}},
  {"bxor2k", OpCode::BXOR2K, {REGISTER, REGISTER, CONSTANT}},
  {"bshl1", OpCode::BSHL1, {REGISTER, REGISTER}},
  {"bshl2", OpCode::BSHL2, {REGISTER, REGISTER, REGISTER}},
  {"bshl1k", OpCode::BSHL1K, {REGISTER, CONSTANT}},
  {"bshl2k", OpCode::BSHL2K, {REGISTER, REGISTER, CONSTANT}},
  {"bshr1", OpCode::BSHR1, {REGISTER, REGISTER}},
  {"bshr2", OpCode::BSHR2, {REGISTER, REGISTER, REGISTER}},
  {"bshr1k", OpCode::BSHR1K, {REGISTER, CONSTANT}},
  {"bshr2k", OpCode::BSHR2K, {REGISTER, REGISTER, CONSTANT}},
  {"bnot", OpCode::BNOT, {REGISTER, REGISTER}},
  {"bnotk", OpCode::BNOTK, {REGISTER, CONSTANT}},
  {"and", OpCode::AND, {REGISTER, REGISTER, REGISTER}},
  {"andk", OpCode::ANDK, {REGISTER, REGISTER, CONSTANT}},
  {"or", OpCode::OR, {REGISTER, REGISTER, REGISTER}},
  {"ork", OpCode::ORK, {REGISTER, REGISTER, CONSTANT}},
  {"ieq", OpCode::IEQ, {REGISTER, REGISTER, REGISTER}},
  {"ieqk", OpCode::IEQK, {REGISTER, REGISTER, CONSTANT}},
  {"feq", OpCode::FEQ, {REGISTER, REGISTER, REGISTER}},
  {"feqk", OpCode::FEQK, {REGISTER, REGISTER, CONSTANT}},
  {"beq", OpCode::BEQ, {REGISTER, REGISTER, REGISTER}},
  {"beqk", OpCode::BEQK, {REGISTER, REGISTER, CONSTANT}},
  {"seq", OpCode::SEQ, {REGISTER, REGISTER, REGISTER}},
  {"seqk", OpCode::SEQK, {REGISTER, REGISTER, CONSTANT}},
  {"ineq", OpCode::INEQ, {REGISTER, REGISTER, REGISTER}},
  {"ineqk", OpCode::INEQK, {REGISTER, REGISTER, CONSTANT}},
  {"fneq", OpCode::FNEQ, {REGISTER, REGISTER, REGISTER}},
  {"fneqk", OpCode::FNEQK, {REGISTER, REGISTER, CONSTANT}},
  {"bneq", OpCode::BNEQ, {REGISTER, REGISTER, REGISTER}},
  {"bneqk", OpCode::BNEQK, {REGISTER, REGISTER, CONSTANT}},
  {"sneq", OpCode::SNEQ, {REGISTER, REGISTER, REGISTER}},
  {"sneqk", OpCode::SNEQK, {REGISTER, REGISTER, CONSTANT}},
  {"is", OpCode::IS, {REGISTER, REGISTER, REGISTER}},
  {"ilt", OpCode::ILT, {REGISTER, REGISTER, REGISTER}},
  {"iltk", OpCode::ILTK, {REGISTER, REGISTER, CONSTANT}},
  {"flt", OpCode::FLT, {REGISTER, REGISTER, REGISTER}},
  {"fltk", OpCode::FLTK, {REGISTER, REGISTER, CONSTANT}},
  {"igt", OpCode::IGT, {REGISTER, REGISTER, REGISTER}},
  {"igtk", OpCode::IGTK, {REGISTER, REGISTER, CONSTANT}},
  {"fgt", OpCode::FGT, {REGISTER, REGISTER, REGISTER}},
  {"fgtk", OpCode::FGTK, {REGISTER, REGISTER, CONSTANT}},
  {"ilteq", OpCode::ILTEQ, {REGISTER, REGISTER, REGISTER}},
  {"ilteqk", OpCode::ILTEQK, {REGISTER, REGISTER, CONSTANT}},
  {"flteq", OpCode::FLTEQ, {REGISTER, REGISTER, REGISTER}},
  {"flteqk", OpCode::FLTEQK, {REGISTER, REGISTER, CONSTANT}},
  {"igteq", OpCode::IGTEQ, {REGISTER, REGISTER, REGISTER}},
  {"igteqk", OpCode::IGTEQK, {REGISTER, REGISTER, CONSTANT}},
  {"fgteq", OpCode::FGTEQ, {REGISTER, REGISTER, REGISTER}},
  {"fgteqk", OpCode::FGTEQK, {REGISTER, REGISTER, CONSTANT}},
  {"not", OpCode::NOT, {REGISTER, REGISTER}},
  {"jmp", OpCode::JMP, {LABEL}},
  {"jmpif", OpCode::JMPIF, {REGISTER, LABEL}},
  {"savesp", OpCode::SAVESP, {}},
  {"restsp", OpCode::RESTSP, {}},
  {"push", OpCode::PUSH, {REGISTER}},
  {"pushk", OpCode::PUSHK, {CONSTANT}},
  {"getarg", OpCode::GETARG, {REGISTER, GENERIC}},
  {"getargref", OpCode::GETARGREF, {REGISTER, GENERIC}},
  {"setarg", OpCode::SETARG, {REGISTER, GENERIC}},
  {"getlocal", OpCode::GETLOCAL, {REGISTER, GENERIC}},
  {"getlocalref", OpCode::GETLOCALREF, {REGISTER, GENERIC}},
  {"setlocal", OpCode::SETLOCAL, {REGISTER, GENERIC}},
  {"btoi", OpCode::BTOI, {REGISTER, REGISTER}},
  {"ftoi", OpCode::FTOI, {REGISTER, REGISTER}},
  {"stoi", OpCode::STOI, {REGISTER, REGISTER}},
  {"itof", OpCode::ITOF, {REGISTER, REGISTER}},
  {"btof", OpCode::BTOF, {REGISTER, REGISTER}},
  {"stof", OpCode::STOF, {REGISTER, REGISTER}},
  {"itob", OpCode::ITOB, {REGISTER, REGISTER}},
  {"stob", OpCode::STOB, {REGISTER, REGISTER}},
  {"itos", OpCode::ITOS, {REGISTER, REGISTER}},
  {"ftos", OpCode::FTOS, {REGISTER, REGISTER}},
  {"btos", OpCode::BTOS, {REGISTER, REGISTER}},
  {"artos", OpCode::ARTOS, {REGISTER, REGISTER}},
  {"dttos", OpCode::DTTOS, {REGISTER, REGISTER}},
  {"fntos", OpCode::FNTOS, {REGISTER, REGISTER}},
  // ...
};

static char getOperandPrefix(Operand kind)
{
  switch (kind) {
    case LABEL:
      return 'L';
    case REGISTER:
      return 'R';
    case CONSTANT:
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
  oss << ansi::format(il->op_str, ansi::Foreground::Magenta,
                      ansi::Background::Black, ansi::Style::Bold)
      << " ";

  for (size_t i = 0; u16 operand : ops) {
    // disgusting addressing hack
    const Operand* ok;
    if ((ok = &(il->ol.a) + i++, *ok == NONE)) {
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
    if (*(ok + 1) != NONE) {
      oss << " ";
    }
  }

  return oss.str();
}
