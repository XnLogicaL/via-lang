// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_INSTRUCTION_H_
#define VIA_CORE_INSTRUCTION_H_

#include <via/config.h>

namespace via {

namespace core {

namespace vm {

// oh boy
enum class Opcode : u16 {
  // meta opcodes
  NOP,        // nop
  HALT,       // halt
  EXTRAARG1,  // extraarg <a: any>
  EXTRAARG2,  // extraarg <a: any> <b: any>
  EXTRAARG3,  // extraarg <a: any> <b: any> <c: any>

  // arithmetic opcodes
  IADD1,    // iadd1 <ra: register<int>> <rb: register<int>>
  IADD2,    // iadd2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  IADD1K,   // iadd1k <ra: register<int>> <k: constant<int>>
  IADD2K,   // iadd2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  FADD1,    // fadd1 <ra: register<float>> <rb: register<float>>
  FADD2,    // fadd2 <ra: register<float>> <rb: register<float>> <rc:
            // register<float>>
  FADD1K,   // fadd1k <ra: register<float>> <k: constant<float>>
  FADD2K,   // fadd2k <ra: register<float>> <rb: register<float>> <k:
            // constant<float>>
  FADD1X,   // fadd1x <ra: register<float>> <rb: register<int>>
  FADD2X,   // fadd2x <ra: register<float>> <rb: register<int>> <rc:
            // register<float>>
  FADD1XK,  // fadd1xk <ra: register<int>> <k: constant<float>>
  FADD2XK,  // fadd2xk <ra: register<float>> <rb: register<int>> <k:
            // constant<float>>
  ISUB1,    // isub1 <ra: register<int>> <rb: register<int>>
  ISUB2,    // isub2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  ISUB1K,   // isub1k <ra: register<int>> <k: constant<int>>
  ISUB2K,   // isub2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  FSUB1,    // fsub1 <ra: register<float>> <rb: register<float>>
  FSUB2,    // fsub2 <ra: register<float>> <rb: register<float>> <rc:
            // register<float>>
  FSUB1K,   // fsub1k <ra: register<float>> <k: register<float>>
  FSUB2K,   // fsub2k <ra: register<float>> <rb: register<float>> <k:
            // register<float>>
  FSUB1X,   // fsub1x <ra: register<float>> <rb: register<int>>
  FSUB2X,   // fsub2x <ra: register<float>> <rb: register<int>> <rc:
            // register<float>>
  FSUB1XK,  // fsub1xk <ra: register<int>> <k: register<float>>
  FSUB2XK,  // fsub2xk <ra: register<float>> <rb: register<int>> <k:
            // register<float>>
  IMUL1,    // imul1 <ra: register<int>> <rb: register<int>>
  IMUL2,    // imul2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  IMUL1K,   // imul1k <ra: register<int>> <k: constant<int>>
  IMUL2K,   // imul2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  FMUL1,    // fmul1 <ra: register<float>> <rb: register<float>>
  FMUL2,    // fmul2 <ra: register<float>> <rb: register<float>> <rc:
            // register<float>>
  FMUL1K,   // fmul1k <ra: register<float>> <k: register<float>>
  FMUL2K,   // fmul2k <ra: register<float>> <rb: register<float>> <k:
            // register<float>>
  FMUL1X,   // fmul1x <ra: register<float>> <rb: register<int>>
  FMUL2X,   // fmul2x <ra: register<float>> <rb: register<int>> <rc:
            // register<float>>
  FMUL1XK,  // fmul1xk <ra: register<int>> <k: register<float>>
  FMUL2XK,  // fmul2xk <ra: register<float>> <rb: register<int>> <k:
            // register<float>>
  IDIV1,    // iadd1 <ra: register<int>> <rb: register<int>>
  IDIV2,    // iadd2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  IDIV1K,   // iadd1k <ra: register<int>> <k: constant<int>>
  IDIV2K,   // iadd2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  FDIV1,    // fadd1 <ra: register<float>> <rb: register<float>>
  FDIV2,    // fadd2 <ra: register<float>> <rb: register<float>> <rc:
            // register<float>>
  FDIV1K,   // fadd1k <ra: register<float>> <k: register<float>>
  FDIV2K,   // fadd2k <ra: register<float>> <rb: register<float>> <k:
            // register<float>>
  FDIV1X,   // fadd1x <ra: register<float>> <rb: register<int>>
  FDIV2X,   // fadd2x <ra: register<float>> <rb: register<int>> <rc:
            // register<float>>
  FDIV1XY,  // fadd1xy <ra: register<int>> <rb: register<float>>
  FDIV2XY,  // fadd2xy <ra: register<float>> <rb: register<float>> <rc:
            // register<int>>
  FDIV1XK,  // fadd1xk <ra: register<int>> <k: register<float>>
  FDIV2XK,  // fadd2xk <ra: register<float>> <rb: register<int>> <k:
            // register<float>>
  FDIV1XYK,  // fadd1xyk <ra: register<float>> <k: constant<int>>
  FDIV2XYK,  // fadd2xyk <ra: register<float>> <rb: register<float>> <k:
             // constant<int>>
  IPOW1,     // ipow1 <ra: register<int>> <rb: register<int>>
  IPOW2,    // ipow2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  IPOW1K,   // ipow1k <ra: register<int>> <k: constant<int>>
  IPOW2K,   // ipow2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  FPOW1,    // fpow1 <ra: register<float>> <rb: register<float>>
  FPOW2,    // fpow2 <ra: register<float>> <rb: register<float>> <rc:
            // register<float>>
  FPOW1K,   // fpow1k <ra: register<float>> <k: register<float>>
  FPOW2K,   // fpow2k <ra: register<float>> <rb: register<float>> <k:
            // register<float>>
  FPOW1X,   // fpow1x <ra: register<float>> <rb: register<int>>
  FPOW2X,   // fpow2x <ra: register<float>> <rb: register<int>> <rc:
            // register<float>>
  FPOW1XK,  // fpow1xk <ra: register<float>> <k: constant<int>>
  FPOW2XK,  // fpow2xk <ra: register<float>> <rb: register<int>> <k:
            // constant<int>>
  FPOW1XY,  // fpow1xy <ra: register<int>> <rb: register<float>>
  FPOW2XY,  // fpow2xy <ra: register<float>> <rb: register<float>> <rc:
            // register<int>>
  FPOW1XYK,  // fpow1xyk <ra: register<float>> <k: constant<int>>
  FPOW2XYK,  // fpow2xyk <ra: register<float>> <rb: register<float>> <k:
             // constant<int>>
  IMOD1,     // imod1 <ra: register<int>> <rb: register<int>>
  IMOD2,    // imod2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  IMOD1K,   // imod1k <ra: register<int>> <k: constant<int>>
  IMOD2K,   // imod2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  FMOD1,    // fmod1 <ra: register<float>> <rb: register<float>>
  FMOD2,    // fmod2 <ra: register<float>> <rb: register<float>> <rc:
            // register<float>>
  FMOD1K,   // fmod1k <ra: register<float>> <k: register<float>>
  FMOD2K,   // fmod2k <ra: register<float>> <rb: register<float>> <k:
            // register<float>>
  FMOD1X,   // fmod1x <ra: register<float>> <rb: register<int>>
  FMOD2X,   // fmod2x <ra: register<float>> <rb: register<int>> <rc:
            // register<float>>
  FMOD1XK,  // fmod1xk <ra: register<float>> <k: constant<int>>
  FMOD2XK,  // fmod2xk <ra: register<float>> <rb: register<int>> <k:
            // constant<int>>
  FMOD1XY,  // fmod1xy <ra: register<int>> <rb: register<float>>
  FMOD2XY,  // fmod2xy <ra: register<float>> <rb: register<float>> <rc:
            // register<int>>
  FMOD1XYK,  // fmod1xyk <ra: register<float>> <k: constant<int>>
  FMOD2XYK,  // fmod2xyk <ra: register<float>> <rb: register<float>> <k:
             // constant<int>>

  // bitwise opcodes
  BAND1,
  BAND2,
  BOR1,
  BOR2,
  BXOR1,
  BXOR2,
  BNOT,
  BSHL,
  BSHR,

  // register opcodes
  MOVE,        // move <dst: register> <src: register>
  XCHG,        // xchg <r0: register> <r1: register>
  COPY,        // copy <dst: register> <src: register>
  COPYREF,     // copyref <dst: register> <src: register>
  LOADTRUE,    // loadtrue <dst: register>
  LOADFALSE,   // loadfalse <dst: register>
  NEWSTR,      // newstring <dst: register>
  NEWSTR2,     // newstring2 <dst: register> <presize: id>
  NEWARR,      // newarray <dst: register>
  NEWARR2,     // newarray2 <dst: register> <presize: id>
  NEWDICT,     // newdict <dst: register>
  NEWTUPLE,    // newtuple <dst: register> <presize: id> ...extraarg1<val:
               // register>
  NEWCLOSURE,  // newclosure <dst: register> <id: constant>

  // comparison opcodes
  NOT,   // not <dst: register> <src: register>
  AND,   // and <dst: register> <lhs: register> <rhs: register>
  OR,    // or <dst: register> <lhs: register> <rhs: register>
  EQ,    // eq <dst: register> <lhs: register> <rhs: register>
  NEQ,   // neq <dst: register> <lhs: register> <rhs: register>
  IS,    // is <dst: register> <lhs: register> <rhs: register>
  LT,    // lt <dst: register> <lhs: register> <rhs: register>
  GT,    // gt <dst: register> <lhs: register> <rhs: register>
  LTEQ,  // lteq <dst: register> <lhs: register> <rhs: register>
  GTEQ,  // gteq <dst: register> <lhs: register> <rhs: register>

  // control flow opcodes
  JMP,       // jmp <lbl: id>
  JMPIF,     // jmpif <cnd: register> <lbl: id>
  JMPIFNOT,  // jmpifnot <cnd: register> <lbl: id>
  JMPIFEQ,   // jmpifeq <lhs: register> <rhs: register> <lbl: id>
  JMPIFIS,   // jmpifis <lhs: register> <rhs: register> <lbl: id>
  JMPIFLT,  // jmpiflt <lhs: register<number>> <rhs: register<number>> <lbl: id>
  JMPIFGT,  // jmpifgt <lhs: register<number>> <rhs: register<number>> <lbl: id>
  JMPIFLTEQ,  // jmpiflteq <lhs: register<number>> <rhs: register<number>> <lbl:
              // id>
  JMPIFGTEQ,  // jmpifgteq <lhs: register<number>> <rhs: register<number>> <lbl:
              // id>

  // stack opcodes
  PUSH,         // push <src: register>
  PUSHK,        // pushk <val: constant>
  GETARG,       // getarg <dst: register> <idx: id>
  GETARGREF,    // getargref <dst: register> <idx: id>
  SETARG,       // setarg <src: register> <idx: id>
  GETLOCAL,     // getlocal <dst: register> <idx: id>
  GETLOCALREF,  // getlocalref <dst: register> <idx: id>
  SETLOCAL,     // setlocal <src: register> <idx: id>
  DUPLOCAL,     // duplocal <id: id>
  DUPLOCALREF,  // duplocalref <id: id>

  // cast opcodes
  ICASTB,       // icastb <dst: register> <bool: register>
  ICASTF,       // icastf <dst: register> <fp: register>
  ICASTSTR,     // icaststr <dst: register> <str: register>
  FCASTI,       // fcasti <dst: register> <int: register>
  FCASTB,       // fcastb <dst: register> <bool: register>
  FCASTSTR,     // fcaststr <dst: register> <str: register>
  BCASTI,       // bcasti <dst: register> <int: register>
  BCASTSTR,     // bcaststr <dst: register> <str: register>
  STRCASTI,     // strcasti <dst: register> <int: register>
  STRCASTF,     // strcastf <dst: register> <fp: register>
  STRCASTB,     // strcastb <dst: register> <bool: register>
  STRCASTARR,   // strcastarr <dst: register> <arr: register>
  STRCASTDICT,  // strcastdict <dst: register> <dict: register>
  STRCASTFUNC,  // strcastfunc <dst: register> <func: register>

  // function opcodes
  CAPTURE,    // capture <stk: id>
  CALL,       // call <callee: register<function>> <argc: id>
  PCALL,      // pcall <callee: register<function>> <argc: id>
  RET,        // ret <val: register>
  RETNIL,     // retnil
  RETTRUE,    // rettrue
  RETFALSE,   // retfalse
  RETK,       // retk <val: constant>
  GETUPV,     // getupv <dst: register> <id: id>
  GETUPVREF,  // getupvref <dst: register> <id: id>
  SETUPV,     // setupv <src: register> <id: id>

  // string opcodes
  STRGET,      // strget <dst: register> <str: register> <idx: id>
  STRSET,      // strset <str: register> <idx: id> <chr: id>
  STRGETLEN,   // strgetlen <dst: register> <str: register>
  STRCONCAT,   // strconcat <dst: register> <lhs: register> <rhs: register>
  STRCONCATK,  // strconcatk <dst: register> <lhs: register> <k: constant>

  // array opcodes
  ARRGET,     // arrget <dst: register> <arr: register> <idx: id>
  ARRSET,     // arrset <src: register> <arr: register> <idx: id>
  ARRGETLEN,  // arrgetlen <dst: register> <arr: register>

  // dict opcodes
  DICTGET,     // dictget <dst: register> <dict: register> <key: register>
  DICTSET,     // dictset <src: register> <dict: register> <key: register>
  DICTGETLEN,  // dictgetlen <dst: register> <dict: register>

  // object opcodes
  NEWINSTANCE,   // newinstance <dst: register> <klass: register>
                 // ...extraarg2<idx, val: register>
  GETSUPER,      // getsuper <dst: register> <inst: register>
  GETSTATIC,     // getstatic <dst: register> <obj: register> <idx: id>
  GETDYNAMIC,    // getdynamic <dst: register> <inst: register> <idx: id>
  SETSTATIC,     // setstatic <src: register> <obj: register> <idx: id>
  SETDYNAMIC,    // setdynamic <src: register> <inst: register> <idx: id>
  CALLSTATIC,    // callstatic <obj: register> <idx: id> <argc: id>
  PCALLSTATIC,   // pcallstatic <obj: register> <idx: id> <argc: id>
  CALLDYNAMIC,   // calldynamic <inst: register> <idx: id> <argc: id>
  PCALLDYNAMIC,  // pcalldynamic <inst: register> <idx: id> <argc: id>
};

Opcode opcode_from_string(const char* str);
const char* opcode_to_string(Opcode opc);

struct Instruction {
  Opcode op = Opcode::NOP;
  u16 a, b, c;
};

String instruction_format(Instruction insn);

}  // namespace vm

}  // namespace core

}  // namespace via

#endif
