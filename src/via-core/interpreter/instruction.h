// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_INSTRUCTION_H_
#define VIA_CORE_INSTRUCTION_H_

#include "common.h"

namespace via {

// oh boy
enum Opcode : u16 {
  // meta opcodes
  VOP_NOP,       // nop
  VOP_HALT,      // halt
  VOP_EXTRAARG1, // extraarg <a: any>
  VOP_EXTRAARG2, // extraarg <a: any> <b: any>
  VOP_EXTRAARG3, // extraarg <a: any> <b: any> <c: any>

  // arithmetic opcodes
  VOP_IADD1,    // iadd1 <ra: register<int>> <rb: register<int>>
  VOP_IADD2,    // iadd2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  VOP_IADD1K,   // iadd1k <ra: register<int>> <k: constant<int>>
  VOP_IADD2K,   // iadd2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  VOP_FADD1,    // fadd1 <ra: register<float>> <rb: register<float>>
  VOP_FADD2,    // fadd2 <ra: register<float>> <rb: register<float>> <rc: register<float>>
  VOP_FADD1K,   // fadd1k <ra: register<float>> <k: constant<float>>
  VOP_FADD2K,   // fadd2k <ra: register<float>> <rb: register<float>> <k: constant<float>>
  VOP_FADD1X,   // fadd1x <ra: register<float>> <rb: register<int>>
  VOP_FADD2X,   // fadd2x <ra: register<float>> <rb: register<int>> <rc: register<float>>
  VOP_FADD1XK,  // fadd1xk <ra: register<int>> <k: constant<float>>
  VOP_FADD2XK,  // fadd2xk <ra: register<float>> <rb: register<int>> <k: constant<float>>
  VOP_ISUB1,    // isub1 <ra: register<int>> <rb: register<int>>
  VOP_ISUB2,    // isub2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  VOP_ISUB1K,   // isub1k <ra: register<int>> <k: constant<int>>
  VOP_ISUB2K,   // isub2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  VOP_FSUB1,    // fsub1 <ra: register<float>> <rb: register<float>>
  VOP_FSUB2,    // fsub2 <ra: register<float>> <rb: register<float>> <rc: register<float>>
  VOP_FSUB1K,   // fsub1k <ra: register<float>> <k: register<float>>
  VOP_FSUB2K,   // fsub2k <ra: register<float>> <rb: register<float>> <k: register<float>>
  VOP_FSUB1X,   // fsub1x <ra: register<float>> <rb: register<int>>
  VOP_FSUB2X,   // fsub2x <ra: register<float>> <rb: register<int>> <rc: register<float>>
  VOP_FSUB1XK,  // fsub1xk <ra: register<int>> <k: register<float>>
  VOP_FSUB2XK,  // fsub2xk <ra: register<float>> <rb: register<int>> <k: register<float>>
  VOP_IMUL1,    // imul1 <ra: register<int>> <rb: register<int>>
  VOP_IMUL2,    // imul2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  VOP_IMUL1K,   // imul1k <ra: register<int>> <k: constant<int>>
  VOP_IMUL2K,   // imul2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  VOP_FMUL1,    // fmul1 <ra: register<float>> <rb: register<float>>
  VOP_FMUL2,    // fmul2 <ra: register<float>> <rb: register<float>> <rc: register<float>>
  VOP_FMUL1K,   // fmul1k <ra: register<float>> <k: register<float>>
  VOP_FMUL2K,   // fmul2k <ra: register<float>> <rb: register<float>> <k: register<float>>
  VOP_FMUL1X,   // fmul1x <ra: register<float>> <rb: register<int>>
  VOP_FMUL2X,   // fmul2x <ra: register<float>> <rb: register<int>> <rc: register<float>>
  VOP_FMUL1XK,  // fmul1xk <ra: register<int>> <k: register<float>>
  VOP_FMUL2XK,  // fmul2xk <ra: register<float>> <rb: register<int>> <k: register<float>>
  VOP_IDIV1,    // iadd1 <ra: register<int>> <rb: register<int>>
  VOP_IDIV2,    // iadd2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  VOP_IDIV1K,   // iadd1k <ra: register<int>> <k: constant<int>>
  VOP_IDIV2K,   // iadd2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  VOP_FDIV1,    // fadd1 <ra: register<float>> <rb: register<float>>
  VOP_FDIV2,    // fadd2 <ra: register<float>> <rb: register<float>> <rc: register<float>>
  VOP_FDIV1K,   // fadd1k <ra: register<float>> <k: register<float>>
  VOP_FDIV2K,   // fadd2k <ra: register<float>> <rb: register<float>> <k: register<float>>
  VOP_FDIV1X,   // fadd1x <ra: register<float>> <rb: register<int>>
  VOP_FDIV2X,   // fadd2x <ra: register<float>> <rb: register<int>> <rc: register<float>>
  VOP_FDIV1XY,  // fadd1xy <ra: register<int>> <rb: register<float>>
  VOP_FDIV2XY,  // fadd2xy <ra: register<float>> <rb: register<float>> <rc: register<int>>
  VOP_FDIV1XK,  // fadd1xk <ra: register<int>> <k: register<float>>
  VOP_FDIV2XK,  // fadd2xk <ra: register<float>> <rb: register<int>> <k: register<float>>
  VOP_FDIV1XYK, // fadd1xyk <ra: register<float>> <k: constant<int>>
  VOP_FDIV2XYK, // fadd2xyk <ra: register<float>> <rb: register<float>> <k: constant<int>>
  VOP_IPOW1,    // ipow1 <ra: register<int>> <rb: register<int>>
  VOP_IPOW2,    // ipow2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  VOP_IPOW1K,   // ipow1k <ra: register<int>> <k: constant<int>>
  VOP_IPOW2K,   // ipow2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  VOP_FPOW1,    // fpow1 <ra: register<float>> <rb: register<float>>
  VOP_FPOW2,    // fpow2 <ra: register<float>> <rb: register<float>> <rc: register<float>>
  VOP_FPOW1K,   // fpow1k <ra: register<float>> <k: register<float>>
  VOP_FPOW2K,   // fpow2k <ra: register<float>> <rb: register<float>> <k: register<float>>
  VOP_FPOW1X,   // fpow1x <ra: register<float>> <rb: register<int>>
  VOP_FPOW2X,   // fpow2x <ra: register<float>> <rb: register<int>> <rc: register<float>>
  VOP_FPOW1XK,  // fpow1xk <ra: register<float>> <k: constant<int>>
  VOP_FPOW2XK,  // fpow2xk <ra: register<float>> <rb: register<int>> <k: constant<int>>
  VOP_FPOW1XY,  // fpow1xy <ra: register<int>> <rb: register<float>>
  VOP_FPOW2XY,  // fpow2xy <ra: register<float>> <rb: register<float>> <rc: register<int>>
  VOP_FPOW1XYK, // fpow1xyk <ra: register<float>> <k: constant<int>>
  VOP_FPOW2XYK, // fpow2xyk <ra: register<float>> <rb: register<float>> <k: constant<int>>
  VOP_IMOD1,    // imod1 <ra: register<int>> <rb: register<int>>
  VOP_IMOD2,    // imod2 <ra: register<int>> <rb: register<int>> <rc: register<int>>
  VOP_IMOD1K,   // imod1k <ra: register<int>> <k: constant<int>>
  VOP_IMOD2K,   // imod2k <ra: register<int>> <rb: register<int>> <k: constant<int>>
  VOP_FMOD1,    // fmod1 <ra: register<float>> <rb: register<float>>
  VOP_FMOD2,    // fmod2 <ra: register<float>> <rb: register<float>> <rc: register<float>>
  VOP_FMOD1K,   // fmod1k <ra: register<float>> <k: register<float>>
  VOP_FMOD2K,   // fmod2k <ra: register<float>> <rb: register<float>> <k: register<float>>
  VOP_FMOD1X,   // fmod1x <ra: register<float>> <rb: register<int>>
  VOP_FMOD2X,   // fmod2x <ra: register<float>> <rb: register<int>> <rc: register<float>>
  VOP_FMOD1XK,  // fmod1xk <ra: register<float>> <k: constant<int>>
  VOP_FMOD2XK,  // fmod2xk <ra: register<float>> <rb: register<int>> <k: constant<int>>
  VOP_FMOD1XY,  // fmod1xy <ra: register<int>> <rb: register<float>>
  VOP_FMOD2XY,  // fmod2xy <ra: register<float>> <rb: register<float>> <rc: register<int>>
  VOP_FMOD1XYK, // fmod1xyk <ra: register<float>> <k: constant<int>>
  VOP_FMOD2XYK, // fmod2xyk <ra: register<float>> <rb: register<float>> <k: constant<int>>

  // bitwise opcodes
  VOP_BAND1,
  VOP_BAND2,
  VOP_BOR1,
  VOP_BOR2,
  VOP_BXOR1,
  VOP_BXOR2,
  VOP_BNOT,
  VOP_BSHL,
  VOP_BSHR,

  // register opcodes
  VOP_MOVE,       // move <dst: register> <src: register>
  VOP_XCHG,       // xchg <r0: register> <r1: register>
  VOP_COPY,       // copy <dst: register> <src: register>
  VOP_COPYREF,    // copyref <dst: register> <src: register>
  VOP_LOADTRUE,   // loadtrue <dst: register>
  VOP_LOADFALSE,  // loadfalse <dst: register>
  VOP_NEWSTR,     // newstring <dst: register>
  VOP_NEWSTR2,    // newstring2 <dst: register> <presize: id>
  VOP_NEWARR,     // newarray <dst: register>
  VOP_NEWARR2,    // newarray2 <dst: register> <presize: id>
  VOP_NEWDICT,    // newdict <dst: register>
  VOP_NEWTUPLE,   // newtuple <dst: register> <presize: id> ...extraarg1<val: register>
  VOP_NEWCLOSURE, // newclosure <dst: register> <id: constant>

  // comparison opcodes
  VOP_NOT,  // not <dst: register> <src: register>
  VOP_AND,  // and <dst: register> <lhs: register> <rhs: register>
  VOP_OR,   // or <dst: register> <lhs: register> <rhs: register>
  VOP_EQ,   // eq <dst: register> <lhs: register> <rhs: register>
  VOP_NEQ,  // neq <dst: register> <lhs: register> <rhs: register>
  VOP_IS,   // is <dst: register> <lhs: register> <rhs: register>
  VOP_LT,   // lt <dst: register> <lhs: register> <rhs: register>
  VOP_GT,   // gt <dst: register> <lhs: register> <rhs: register>
  VOP_LTEQ, // lteq <dst: register> <lhs: register> <rhs: register>
  VOP_GTEQ, // gteq <dst: register> <lhs: register> <rhs: register>

  // control flow opcodes
  VOP_JMP,       // jmp <lbl: id>
  VOP_JMPIF,     // jmpif <cnd: register> <lbl: id>
  VOP_JMPIFNOT,  // jmpifnot <cnd: register> <lbl: id>
  VOP_JMPIFEQ,   // jmpifeq <lhs: register> <rhs: register> <lbl: id>
  VOP_JMPIFIS,   // jmpifis <lhs: register> <rhs: register> <lbl: id>
  VOP_JMPIFLT,   // jmpiflt <lhs: register<number>> <rhs: register<number>> <lbl: id>
  VOP_JMPIFGT,   // jmpifgt <lhs: register<number>> <rhs: register<number>> <lbl: id>
  VOP_JMPIFLTEQ, // jmpiflteq <lhs: register<number>> <rhs: register<number>> <lbl: id>
  VOP_JMPIFGTEQ, // jmpifgteq <lhs: register<number>> <rhs: register<number>> <lbl: id>

  // stack opcodes
  VOP_PUSH,        // push <src: register>
  VOP_PUSHK,       // pushk <val: constant>
  VOP_GETARG,      // getarg <dst: register> <idx: id>
  VOP_GETARGREF,   // getargref <dst: register> <idx: id>
  VOP_SETARG,      // setarg <src: register> <idx: id>
  VOP_GETLOCAL,    // getlocal <dst: register> <idx: id>
  VOP_GETLOCALREF, // getlocalref <dst: register> <idx: id>
  VOP_SETLOCAL,    // setlocal <src: register> <idx: id>
  VOP_DUPLOCAL,    // duplocal <id: id>
  VOP_DUPLOCALREF, // duplocalref <id: id>

  // cast opcodes
  VOP_ICASTB,      // icastb <dst: register> <bool: register>
  VOP_ICASTF,      // icastf <dst: register> <fp: register>
  VOP_ICASTSTR,    // icaststr <dst: register> <str: register>
  VOP_FCASTI,      // fcasti <dst: register> <int: register>
  VOP_FCASTB,      // fcastb <dst: register> <bool: register>
  VOP_FCASTSTR,    // fcaststr <dst: register> <str: register>
  VOP_BCASTI,      // bcasti <dst: register> <int: register>
  VOP_BCASTSTR,    // bcaststr <dst: register> <str: register>
  VOP_STRCASTI,    // strcasti <dst: register> <int: register>
  VOP_STRCASTF,    // strcastf <dst: register> <fp: register>
  VOP_STRCASTB,    // strcastb <dst: register> <bool: register>
  VOP_STRCASTARR,  // strcastarr <dst: register> <arr: register>
  VOP_STRCASTDICT, // strcastdict <dst: register> <dict: register>
  VOP_STRCASTFUNC, // strcastfunc <dst: register> <func: register>

  // function opcodes
  VOP_CAPTURE,   // capture <stk: id>
  VOP_CALL,      // call <callee: register<function>> <argc: id>
  VOP_PCALL,     // pcall <callee: register<function>> <argc: id>
  VOP_RET,       // ret <val: register>
  VOP_RETNIL,    // retnil
  VOP_RETTRUE,   // rettrue
  VOP_RETFALSE,  // retfalse
  VOP_RETK,      // retk <val: constant>
  VOP_GETUPV,    // getupv <dst: register> <id: id>
  VOP_GETUPVREF, // getupvref <dst: register> <id: id>
  VOP_SETUPV,    // setupv <src: register> <id: id>

  // string opcodes
  VOP_STRGET,     // strget <dst: register> <str: register> <idx: id>
  VOP_STRSET,     // strset <str: register> <idx: id> <chr: id>
  VOP_STRGETLEN,  // strgetlen <dst: register> <str: register>
  VOP_STRCONCAT,  // strconcat <dst: register> <lhs: register> <rhs: register>
  VOP_STRCONCATK, // strconcatk <dst: register> <lhs: register> <k: constant>

  // array opcodes
  VOP_ARRGET,    // arrget <dst: register> <arr: register> <idx: id>
  VOP_ARRSET,    // arrset <src: register> <arr: register> <idx: id>
  VOP_ARRGETLEN, // arrgetlen <dst: register> <arr: register>

  // dict opcodes
  VOP_DICTGET,    // dictget <dst: register> <dict: register> <key: register>
  VOP_DICTSET,    // dictset <src: register> <dict: register> <key: register>
  VOP_DICTGETLEN, // dictgetlen <dst: register> <dict: register>

  // object opcodes
  VOP_NEWINSTANCE, // newinstance <dst: register> <klass: register> ...extraarg2<idx, val: register>
  VOP_GETSUPER,    // getsuper <dst: register> <inst: register>
  VOP_GETSTATIC,   // getstatic <dst: register> <obj: register> <idx: id>
  VOP_GETDYNAMIC,  // getdynamic <dst: register> <inst: register> <idx: id>
  VOP_SETSTATIC,   // setstatic <src: register> <obj: register> <idx: id>
  VOP_SETDYNAMIC,  // setdynamic <src: register> <inst: register> <idx: id>
  VOP_CALLSTATIC,  // callstatic <obj: register> <idx: id> <argc: id>
  VOP_PCALLSTATIC, // pcallstatic <obj: register> <idx: id> <argc: id>
  VOP_CALLDYNAMIC, // calldynamic <inst: register> <idx: id> <argc: id>
  VOP_PCALLDYNAMIC, // pcalldynamic <inst: register> <idx: id> <argc: id>
};

Opcode opcode_from_string(const char* str);
const char* opcode_to_string(Opcode opc);

struct Instruction {
  Opcode op = VOP_NOP;
  u16 a, b, c;
};

String instruction_format(Instruction insn);

} // namespace via

#endif
