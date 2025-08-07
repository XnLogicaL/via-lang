// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_INSTRUCTION_H_
#define VIA_CORE_INSTRUCTION_H_

#include <via/config.h>

namespace via {

namespace core {

namespace vm {

// special operand symbols:
// [*T] register containing T
// [$T] constant containing T
enum class Opcode : u16 {
  // meta opcodes
  NOP,        // nop
  HALT,       // halt
  EXTRAARG1,  // extraarg <a: any>
  EXTRAARG2,  // extraarg <a: any> <b: any>
  EXTRAARG3,  // extraarg <a: any> <b: any> <c: any>

  // register manip opcodes
  MOVE,        // move        <dst: *> <src: *>
  XCHG,        // xchg        <r0: *>  <r1: *>
  COPY,        // copy        <dst: *> <src: *>
  COPYREF,     // copyref     <dst: *> <src: *>
  LOADTRUE,    // loadtrue    <dst: *>
  LOADFALSE,   // loadfalse   <dst: *>
  NEWSTR,      // newstr      <dst: *>
  NEWSTR2,     // newstr2     <dst: *> <presize>
  NEWARR,      // newarr      <dst: *>
  NEWARR2,     // newarr2     <dst: *> <presize>
  NEWDICT,     // newdict     <dst: *>
  NEWTUPLE,    // newtuple    <dst: *> <presize>  ...extraarg1<val: *>
  NEWCLOSURE,  // newclosure  <dst: *> <id: $str>

  // arithmetic opcodes
  IADD1,    // iadd1    <dst: *int> <src: *int>
  IADD2,    // iadd2    <dst: *int> <lhs: *int> <rhs: *int>
  IADD1K,   // iadd1k   <dst: *int> <src: $int>
  IADD2K,   // iadd2k   <dst: *int> <src: *int> <rhs: $int>
  FADD1,    // fadd1    <dst: *fp>  <src: *fp>
  FADD2,    // fadd2    <dst: *fp>  <lhs: *fp>  <rhs: *fp>
  FADD1K,   // fadd1k   <dst: *fp>  <src: $fp>
  FADD2K,   // iadd2k   <dst: *fp>  <src: *fp>  <rhs: $fp>
  ISUB1,    // isub1    <dst: *int> <src: *int>
  ISUB2,    // isub2    <dst: *int> <lhs: *int> <rhs: *int>
  ISUB1K,   // isub1k   <dst: *int> <src: $int>
  ISUB2K,   // isub2k   <dst: *int> <src: *int> <rhs: $int>
  ISUB2KX,  // isub2kx  <dst: *int> <src: $int> <rhs: *int>
  FSUB1,    // fsub1    <dst: *fp>  <src: *fp>
  FSUB2,    // fsub2    <dst: *fp>  <lhs: *fp>  <rhs: *fp>
  FSUB1K,   // fsub1k   <dst: *fp>  <src: $fp>
  FSUB2K,   // isub2k   <dst: *fp>  <src: *fp>  <rhs: $fp>
  FSUB2KX,  // fsub2kx  <dst: *fp>  <src: $fp>  <rhs: *fp>
  IMUL1,    // imul1    <dst: *int> <src: *int>
  IMUL2,    // imul2    <dst: *int> <lhs: *int> <rhs: *int>
  IMUL1K,   // imul1k   <dst: *int> <src: $int>
  IMUL2K,   // imul2k   <dst: *int> <src: *int> <rhs: $int>
  FMUL1,    // fmul1    <dst: *fp>  <src: *fp>
  FMUL2,    // fmul2    <dst: *fp>  <lhs: *fp>  <rhs: *fp>
  FMUL1K,   // fmul1k   <dst: *fp>  <src: $fp>
  FMUL2K,   // imul2k   <dst: *fp>  <src: *fp>  <rhs: $fp>
  IDIV1,    // idiv1    <dst: *int> <src: *int>
  IDIV2,    // idiv2    <dst: *int> <lhs: *int> <rhs: *int>
  IDIV1K,   // idiv1k   <dst: *int> <src: $int>
  IDIV2K,   // idiv2k   <dst: *int> <src: *int> <rhs: $int>
  IDIV2KX,  // idiv2kx  <dst: *int> <src: $int> <rhs: *int>
  FDIV1,    // fdiv1    <dst: *fp>  <src: *fp>
  FDIV2,    // fdiv2    <dst: *fp>  <lhs: *fp>  <rhs: *fp>
  FDIV1K,   // fdiv1k   <dst: *fp>  <src: $fp>
  FDIV2K,   // idiv2k   <dst: *fp>  <src: *fp>  <rhs: $fp>
  FDIV2KX,  // fdiv2kx  <dst: *fp>  <src: $fp>  <rhs: *fp>
  IPOW1,    // ipow1    <dst: *int> <src: *int>
  IPOW2,    // ipow2    <dst: *int> <lhs: *int> <rhs: *int>
  IPOW1K,   // ipow1k   <dst: *int> <src: $int>
  IPOW2K,   // ipow2k   <dst: *int> <src: *int> <rhs: $int>
  IPOW2KX,  // ipow2kx  <dst: *int> <src: $int> <rhs: *int>
  FPOW1,    // fpow1    <dst: *fp>  <src: *fp>
  FPOW2,    // fpow2    <dst: *fp>  <lhs: *fp>  <rhs: *fp>
  FPOW1K,   // fpow1k   <dst: *fp>  <src: $fp>
  FPOW2K,   // ipow2k   <dst: *fp>  <src: *fp>  <rhs: $fp>
  FPOW2KX,  // fpow2kx  <dst: *fp>  <src: $fp>  <rhs: *fp>
  IMOD1,    // imod1    <dst: *int> <src: *int>
  IMOD2,    // imod2    <dst: *int> <lhs: *int> <rhs: *int>
  IMOD1K,   // imod1k   <dst: *int> <src: $int>
  IMOD2K,   // imod2k   <dst: *int> <src: *int> <rhs: $int>
  IMOD2KX,  // imod2kx  <dst: *int> <src: $int> <rhs: *int>
  FMOD1,    // fmod1    <dst: *fp>  <src: *fp>
  FMOD2,    // fmod2    <dst: *fp>  <lhs: *fp>  <rhs: *fp>
  FMOD1K,   // fmod1k   <dst: *fp>  <src: $fp>
  FMOD2K,   // imod2k   <dst: *fp>  <src: *fp>  <rhs: $fp>
  FMOD2KX,  // fmod2kx  <dst: *fp>  <src: $fp>  <rhs: *fp>
  INEG,
  INEGK,
  FNEG,
  FNEGK,

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

  // comparison opcodes
  NOT,   // not   <dst: *> <src: *>
  AND,   // and   <dst: *> <lhs: *> <rhs: *>
  OR,    // or    <dst: *> <lhs: *> <rhs: *>
  EQ,    // eq    <dst: *> <lhs: *> <rhs: *>
  NEQ,   // neq   <dst: *> <lhs: *> <rhs: *>
  IS,    // is    <dst: *> <lhs: *> <rhs: *>
  LT,    // lt    <dst: *> <lhs: *> <rhs: *>
  GT,    // gt    <dst: *> <lhs: *> <rhs: *>
  LTEQ,  // lteq  <dst: *> <lhs: *> <rhs: *>
  GTEQ,  // gteq  <dst: *> <lhs: *> <rhs: *>

  // control flow opcodes
  JMP,         // jmp        <lbl>
  JMPIF,       // jmpif      <cnd: *>  <lbl>
  JMPIFNOT,    // jmpifnot   <cnd: *>  <lbl>
  JMPIFEQ,     // jmpifeq    <lhs: *>  <rhs: *>      <lbl>
  JMPIFIS,     // jmpifis    <lhs: *>  <rhs: *>      <lbl>
  JMPIFLTI,    // jmpiflt    <lhs: *int> <rhs: *int> <lbl>
  JMPIFLTF,    // jmpiflt    <lhs: *fp>  <rhs: *fp>  <lbl>
  JMPIFGTI,    // jmpifgt    <lhs: *int> <rhs: *int> <lbl>
  JMPIFGTF,    // jmpiflt    <lhs: *fp>  <rhs: *fp>  <lbl>
  JMPIFLTEQI,  // jmpiflteq  <lhs: *int> <rhs: *int> <lbl>
  JMPIFLTEQF,  // jmpiflteq  <lhs: *fp>  <rhs: *fp>  <lbl>
  JMPIFGTEQI,  // jmpifgteq  <lhs: *int> <rhs: *int> <lbl>
  JMPIFGTEQF,  // jmpifgteq  <lhs: *fp>  <rhs: *fp>  <lbl>

  // stack opcodes
  PUSH,         // push         <src: *>
  PUSHK,        // pushk        <val: $>
  GETARG,       // getarg       <dst: *> <idx>
  GETARGREF,    // getargref    <dst: *> <idx>
  SETARG,       // setarg       <src: *> <idx>
  GETLOCAL,     // getlocal     <dst: *> <idx>
  GETLOCALREF,  // getlocalref  <dst: *> <idx>
  SETLOCAL,     // setlocal     <src: *> <idx>
  DUPLOCAL,     // duplocal     <id>
  DUPLOCALREF,  // duplocalref  <id>

  // cast opcodes
  BTOI,   // btoi   <dst: *> <bool: *>
  FTOI,   // ftoi   <dst: *> <fp: *>
  STOI,   // stoi   <dst: *> <str: *>
  ITOF,   // itof   <dst: *> <int: *>
  BTOF,   // btof   <dst: *> <bool: *>
  STOF,   // stof   <dst: *> <str: *>
  ITOB,   // itob   <dst: *> <int: *>
  STOB,   // stob   <dst: *> <str: *>
  ITOS,   // itos   <dst: *> <int: *>
  FTOS,   // ftos   <dst: *> <fp: *>
  BTOS,   // btos   <dst: *> <bool: *>
  ARTOS,  // artos  <dst: *> <arr: *>
  DTTOS,  // dttos  <dst: *> <dict: *>
  FNTOS,  // fntos  <dst: *> <func: *>

  // function opcodes
  CAPTURE,    // capture  <stk>
  CALL,       // call     <callee: *function> <argc>
  PCALL,      // pcall    <callee: *function> <argc>
  RET,        // ret      <val: *>
  RETNIL,     // retnil
  RETTRUE,    // rettrue
  RETFALSE,   // retfalse
  RETK,       // retk       <val: $>
  GETUPV,     // getupv     <dst: *> <id>
  GETUPVREF,  // getupvref  <dst: *> <id>
  SETUPV,     // setupv     <src: *> <id>

  // array opcodes
  ARRGET,     // arrget <dst: *> <arr: *> <idx>
  ARRSET,     // arrset <src: *> <arr: *> <idx>
  ARRGETLEN,  // arrgetlen <dst: *> <arr: *>

  // dict opcodes
  DICTGET,     // dictget <dst: *> <dict: *> <key: *>
  DICTSET,     // dictset <src: *> <dict: *> <key: *>
  DICTGETLEN,  // dictgetlen <dst: *> <dict: *>

  // object opcodes
  NEWINSTANCE,   // newinstance <dst: *> <klass: *>
                 // ...extraarg2<idx, val: *>
  GETSUPER,      // getsuper <dst: *> <inst: *>
  GETSTATIC,     // getstatic <dst: *> <obj: *> <idx>
  GETDYNAMIC,    // getdynamic <dst: *> <inst: *> <idx>
  SETSTATIC,     // setstatic <src: *> <obj: *> <idx>
  SETDYNAMIC,    // setdynamic <src: *> <inst: *> <idx>
  CALLSTATIC,    // callstatic <obj: *> <idx> <argc>
  PCALLSTATIC,   // pcallstatic <obj: *> <idx> <argc>
  CALLDYNAMIC,   // calldynamic <inst: *> <idx> <argc>
  PCALLDYNAMIC,  // pcalldynamic <inst: *> <idx> <argc>
};

Opcode opcode_from_string(const char* str);
const char* opcode_to_string(Opcode opc);

struct Instruction {
  Opcode op = Opcode::NOP;
  u16 a, b, c;

  String to_string() const;
};

}  // namespace vm

}  // namespace core

}  // namespace via

#endif
