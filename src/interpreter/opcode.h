// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_OPCODE_H
#define VIA_HAS_HEADER_OPCODE_H

#include "common.h"

namespace via {

enum class IOpCode : uint16_t {
  // nop
  // No operation.
  NOP,

  // lbl
  // o0: <label_id :: int>
  // Maps the current program counter to <label_id>.
  LBL,

  // exit
  // o0: <exit_code :: int>
  // Safely terminates the VM with exit code <exit_code>.
  EXIT,

  // add
  // o0: <left :: reg(int|float)>
  // o1: <right :: reg(int|float)>
  // Adds the value in register <right> to register <left>.
  ADD,

  // addi
  // o0: <left :: reg(int|float)>
  // o1: <right :: int>
  // Adds the literal Int value <right> to register <left>.
  ADDI,

  // addf
  // o0: <left :: reg(int|float)>
  // o1: <right :: float>
  // Adds the literal float value <right> to register <left>.
  ADDF,

  // sub
  // o0: <left :: reg(int|float)>
  // o1: <right :: reg(int|float)>
  // Subtracts the value in register <right> from register <left>.
  SUB,

  // subi
  // o0: <left :: reg(int|float)>
  // o1: <right :: int>
  // Subtracts the literal Int value <right> from register <left>.
  SUBI,

  // subf
  // o0: <left :: reg(int|float)>
  // o1: <right :: float>
  // Subtracts the literal float value <right> from register <left>.
  SUBF,

  // mul
  // o0: <left :: reg(int|float)>
  // o1: <right :: reg(int|float)>
  // Multiplies the value in register <left> with the value in register <right>.
  MUL,

  // subi
  // o0: <left :: reg(int|float)>
  // o1: <right :: int>
  // Multiplies the value in register <left> with the literal Int value <right>.
  MULI,

  // mulf
  // o0: <left :: reg(int|float)>
  // o1: <right :: float>
  // Multiplies the value in register <left> with the literal float value <right>.
  MULF,

  // div
  // o0: <left :: reg(int|float)>
  // o1: <right :: reg(int|float)>
  // Divides the value in register <left> with the value in register <right>.
  DIV,

  // divi
  // o0: <left :: reg(int|float)>
  // o1: <right :: int>
  // Divides the value in register <left> with the literal Int value <right>
  DIVI,

  // divf
  // o0: <left :: reg(int|float)>
  // o1: <right :: float>
  // Divides the value in register <left> with the literal float value <right>.
  DIVF,

  // mod
  // o0: <left :: reg(int|float)>
  // o1: <right :: reg(int|float)>
  // Performs a modulus operation on the values in register <left> and register <right>.
  MOD,

  // modi
  // o0: <left :: reg(int|float)>
  // o1: <right :: int>
  // Performs a modulus operation on the value in register <left> and the literal Int value
  // <right>.
  MODI,

  // modf
  // o0: <left :: reg(int|float)>
  // o1: <right :: float>
  // Adds the literal float value <right> to register <left>.
  MODF,

  // pow
  // o0: <left :: reg(int|float)>
  // o1: <right :: reg(int|float)>
  // Performs an exponentiation operation with the value in register <left> as base and with the
  // value in register <right> as the power.
  POW,

  // powi
  // o0: <left :: reg(int|float)>
  // o1: <right :: int>
  // Performs an exponentiation operation with the value in register <left> as base and with the
  // literal Int value <right> as the power.
  POWI,

  // powf
  // o0: <left :: reg(int|float)>
  // o1: <right :: float>
  // Performs an exponentiation operation with the value in register <left> as base and with the
  // literal float value <right> as the power.
  POWF,

  // neg
  // o0: <left :: reg(int|float)>
  // Negates the value in o0.
  NEG,

  // mov
  // o0: <dst :: reg(any)>
  // o1: <src :: reg(any)>
  // Copies the value in register <src> into register <dst>.
  MOV,

  // loadk
  // o0: <dst :: reg(any)>
  // o1: <kid :: int>
  // Loads constant <kid> from the constant table into register <dst>.
  LOADK,

  // loadnil
  // o0: <dst :: reg(any)>
  // Loads a Nil value into register <dst>.
  LOADNIL,

  // loadi
  // o0: <dst :: reg(any)>
  // o1: <high :: bytes>
  // o2: <low :: bytes>
  // Interprets <high> and <low> as an Int and loads it into register <dst>.
  LOADI,

  // loadf
  // o0: <dst :: reg(any)>
  // o1: <high :: bytes>
  // o2: <low :: bytes>
  // Interprets <high> and <low> as a float and loads it into register <dst>.
  LOADF,

  // loadbt
  // o0: <dst :: reg(any)>
  // Loads Bool true into register <dst>.
  LOADBT,

  // loadbf
  // o0: <dst :: reg(any)>
  // Loads Bool false into register <dst>.
  LOADBF,

  // loadarr
  // o0: <dst :: reg(any)>
  // Loads an empty array into register <dst>.
  LOADARR,

  // loaddict
  // o0: <dst :: reg(any)>
  // Loads an empty dictionary into register <dst>.
  LOADDICT,

  // closure
  // o0: <dst :: reg(any)>
  // o1: <argc :: int>
  // o2: <insnc :: int>
  // Captures <insnc> amount of instructions starting from itself, creates a closure and loads it
  // into <dst>. Saves the argument stack pointer and captures arguments from [sp..sp+argc].
  CLOSURE,

  // push
  // o0: <src :: reg(any)>
  // Pushes the value in register <src> onto the local stack.
  PUSH,

  // pushk
  // o0: <kid :: int>
  // Pushes constant <kid> from the constant table onto the local stack.
  PUSHK,

  // pushnil
  // Pushes Nil onto the local stack.
  PUSHNIL,

  // pushi
  // o0: <high :: bytes>
  // o1: <low :: bytes>
  // Interprets <high> and <low> as an Int literal and pushes it onto the local stack.
  PUSHI,

  // pushf
  // o0: <high :: bytes>
  // o1: <low :: bytes>
  // Interprets <high> and <low> as a float literal and pushes it onto the local stack.
  PUSHF,

  // pushbt
  // Pushes Bool true onto the stack.
  PUSHBT,

  // pushbf
  // Pushes Bool false onto the stack.
  PUSHBF,

  // drop
  // Drops the top-most value from the local stack.
  DROP,

  GETGLOBAL,
  SETGLOBAL,

  SETUPV,
  GETUPV,
  GETLOCAL,
  SETLOCAL,

  CAPTURE,

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
  RET,
  RET1,
  RET0,
  RETNIL,
  RETGET,

  RAISE,
  TRY,
  CATCH,

  GETARR,
  SETARR,
  NEXTARR,
  LENARR,

  GETDICT,
  SETDICT,
  NEXTDICT,
  LENDICT,

  CONSTR,
  GETSTR,
  SETSTR,
  LENSTR,

  ICAST,
  FCAST,
  STRCAST,
  BCAST,
};
}

#endif
