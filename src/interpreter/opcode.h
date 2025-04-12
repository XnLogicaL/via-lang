//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_OPCODE_H
#define VIA_HAS_HEADER_OPCODE_H

#include "common.h"

namespace via {

enum class IOpCode : uint16_t {
  NOP,
  LBL,
  EXIT,

  ADD,
  ADDI,
  ADDF,

  // ADD2I,
  // ADD2F,
  // ADDFI,

  SUB,
  SUBI,
  SUBF,

  MUL,
  MULI,
  MULF,

  DIV,
  DIVI,
  DIVF,

  MOD,
  MODI,
  MODF,

  POW,
  POWI,
  POWF,

  NEG,

  MOV,

  LOADK,
  LOADNIL,
  LOADI,
  LOADF,
  LOADBT,
  LOADBF,
  NEWARR,
  NEWDICT,
  NEWCLSR,

  PUSH,
  PUSHK,
  PUSHNIL,
  PUSHI,
  PUSHF,
  PUSHBT,
  PUSHBF,
  POP,
  DROP,
  STKGET,
  STKSET,
  ARGGET,

  GGET,
  GSET,

  UPVSET,
  UPVGET,

  LOCALGET,
  LOCALSET,

  INC,
  DEC,
  EQ,
  NEQ,
  AND,
  OR,
  NOT,
  LT,
  GT,
  LTEQ,
  GTEQ,

  JMP,
  JMPIF,
  JMPIFN,
  JMPIFEQ,
  JMPIFNEQ,
  JMPIFLT,
  JMPIFGT,
  JMPIFLTEQ,
  JMPIFGTEQ,

  LJMP,
  LJMPIF,
  LJMPIFN,
  LJMPIFEQ,
  LJMPIFNEQ,
  LJMPIFLT,
  LJMPIFGT,
  LJMPIFLTEQ,
  LJMPIFGTEQ,

  CALL,
  CCALL,
  NTVCALL,
  MTDCALL,
  RET,
  RETNIL,

  RAISE,
  TRY,
  CATCH,

  ARRGET,
  ARRSET,
  ARRNEXT,
  ARRLEN,

  DICTGET,
  DICTSET,
  DICTNEXT,
  DICTLEN,

  STRCON,
  STRGET,
  STRSET,
  STRLEN,

  CASTI,
  CASTF,
  CASTSTR,
  CASTB,
};

}

#endif
