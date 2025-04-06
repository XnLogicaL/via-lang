// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_OPCODE_H
#define VIA_HAS_HEADER_OPCODE_H

#include "common.h"

namespace via {

enum class opcode : uint16_t {
  NOP,
  LBL,
  EXIT,

  ADD,
  ADDK,
  ADDI,
  ADDF,

  SUB,
  SUBK,
  SUBI,
  SUBF,

  MUL,
  MULK,
  MULI,
  MULF,

  DIV,
  DIVK,
  DIVI,
  DIVF,

  MOD,
  MODK,
  MODI,
  MODF,

  POW,
  POWK,
  POWI,
  POWF,

  NEG,

  MOVE,
  SWAP,

  LOADK,
  LOADNIL,
  LOADI,
  LOADF,
  LOADBT,
  LOADBF,
  NEWTBL,
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

  TBLGET,
  TBLSET,
  TBLNEXT,
  TBLLEN,

  STRCONCAT,
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
