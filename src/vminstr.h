// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_INSTRUCTION_H
#define VIA_INSTRUCTION_H

#include "common.h"

namespace via {

// oh boy
enum Opcode : u16 {
  // meta opcodes
  VOP_NOP,
  VOP_HALT,
  VOP_EXTRAARG,

  // arithmetic opcodes
  VOP_IADD1,    // iadd1 <ra: int> <rb: int>
  VOP_IADD2,    // iadd2 <ra: int> <rb: int> <rc: int>
  VOP_IADD1K,   // iadd1k <ra: int> <k0: int>
  VOP_IADD2K,   // iadd2k <ra: int> <rb: int> <k0: int>
  VOP_FADD1,    // fadd1 <ra: float> <rb: float>
  VOP_FADD2,    // fadd2 <ra: float> <rb: float> <rc: float>
  VOP_FADD1K,   // fadd1k <ra: float> <k0: float>
  VOP_FADD2K,   // fadd2k <ra: float> <rb: float> <k0: float>
  VOP_FADD1X,   // fadd1x <ra: float> <rb: int>
  VOP_FADD2X,   // fadd2x <ra: float> <rb: int> <rc: float>
  VOP_FADD1XK,  // fadd1xk <ra: int> <k0: float>
  VOP_FADD2XK,  // fadd2xk <ra: float> <rb: int> <k0: float>
  VOP_ISUB1,    // isub1 <ra: int> <rb: int>
  VOP_ISUB2,    // isub2 <ra: int> <rb: int> <rc: int>
  VOP_ISUB1K,   // isub1k <ra: int> <k0: int>
  VOP_ISUB2K,   // isub2k <ra: int> <rb: int> <k0: int>
  VOP_FSUB1,    // fsub1 <ra: float> <rb: float>
  VOP_FSUB2,    // fsub2 <ra: float> <rb: float> <rc: float>
  VOP_FSUB1K,   // fsub1k <ra: float> <k0: float>
  VOP_FSUB2K,   // fsub2k <ra: float> <rb: float> <k0: float>
  VOP_FSUB1X,   // fsub1x <ra: float> <rb: int>
  VOP_FSUB2X,   // fsub2x <ra: float> <rb: int> <rc: float>
  VOP_FSUB1XK,  // fsub1xk <ra: int> <k0: float>
  VOP_FSUB2XK,  // fsub2xk <ra: float> <rb: int> <k0: float>
  VOP_IMUL1,    // imul1 <ra: int> <rb: int>
  VOP_IMUL2,    // imul2 <ra: int> <rb: int> <rc: int>
  VOP_IMUL1K,   // imul1k <ra: int> <k0: int>
  VOP_IMUL2K,   // imul2k <ra: int> <rb: int> <k0: int>
  VOP_FMUL1,    // fmul1 <ra: float> <rb: float>
  VOP_FMUL2,    // fmul2 <ra: float> <rb: float> <rc: float>
  VOP_FMUL1K,   // fmul1k <ra: float> <k0: float>
  VOP_FMUL2K,   // fmul2k <ra: float> <rb: float> <k0: float>
  VOP_FMUL1X,   // fmul1x <ra: float> <rb: int>
  VOP_FMUL2X,   // fmul2x <ra: float> <rb: int> <rc: float>
  VOP_FMUL1XK,  // fmul1xk <ra: int> <k0: float>
  VOP_FMUL2XK,  // fmul2xk <ra: float> <rb: int> <k0: float>
  VOP_IDIV1,    // iadd1 <ra: int> <rb: int>
  VOP_IDIV2,    // iadd2 <ra: int> <rb: int> <rc: int>
  VOP_IDIV1K,   // iadd1k <ra: int> <k0: int>
  VOP_IDIV2K,   // iadd2k <ra: int> <rb: int> <k0: int>
  VOP_FDIV1,    // fadd1 <ra: float> <rb: float>
  VOP_FDIV2,    // fadd2 <ra: float> <rb: float> <rc: float>
  VOP_FDIV1K,   // fadd1k <ra: float> <k0: float>
  VOP_FDIV2K,   // fadd2k <ra: float> <rb: float> <k0: float>
  VOP_FDIV1X,   // fadd1x <ra: float> <rb: int>
  VOP_FDIV2X,   // fadd2x <ra: float> <rb: int> <rc: float>
  VOP_FDIV1XY,  // fadd1xy <ra: int> <rb: float>
  VOP_FDIV2XY,  // fadd2xy <ra: float> <rb: float> <rc: int>
  VOP_FDIV1XK,  // fadd1xk <ra: int> <k0: float>
  VOP_FDIV2XK,  // fadd2xk <ra: float> <rb: int> <k0: float>
  VOP_FDIV1XYK, // fadd1xyk <ra: float> <k0: int>
  VOP_FDIV2XYK, // fadd2xyk <ra: float> <rb: float> <k0: int>
  VOP_IPOW1,    // ipow1 <ra: int> <rb: int>
  VOP_IPOW2,    // ipow2 <ra: int> <rb: int> <rc: int>
  VOP_IPOW1K,   // ipow1k <ra: int> <k0: int>
  VOP_IPOW2K,   // ipow2k <ra: int> <rb: int> <k0: int>
  VOP_FPOW1,    // fpow1 <ra: float> <rb: float>
  VOP_FPOW2,    // fpow2 <ra: float> <rb: float> <rc: float>
  VOP_FPOW1K,   // fpow1k <ra: float> <k0: float>
  VOP_FPOW2K,   // fpow2k <ra: float> <rb: float> <k0: float>
  VOP_FPOW1X,   // fpow1x <ra: float> <rb: int>
  VOP_FPOW2X,   // fpow2x <ra: float> <rb: int> <rc: float>
  VOP_FPOW1XK,  // fpow1xk <ra: float> <k0: int>
  VOP_FPOW2XK,  // fpow2xk <ra: float> <rb: int> <k0: int>
  VOP_FPOW1XY,  // fpow1xy <ra: int> <rb: float>
  VOP_FPOW2XY,  // fpow2xy <ra: float> <rb: float> <rc: int>
  VOP_FPOW1XYK, // fpow1xyk <ra: float> <k0: int>
  VOP_FPOW2XYK, // fpow2xyk <ra: float> <rb: float> <k0: int>
  VOP_IMOD1,    // imod1 <ra: int> <rb: int>
  VOP_IMOD2,    // imod2 <ra: int> <rb: int> <rc: int>
  VOP_IMOD1K,   // imod1k <ra: int> <k0: int>
  VOP_IMOD2K,   // imod2k <ra: int> <rb: int> <k0: int>
  VOP_FMOD1,    // fmod1 <ra: float> <rb: float>
  VOP_FMOD2,    // fmod2 <ra: float> <rb: float> <rc: float>
  VOP_FMOD1K,   // fmod1k <ra: float> <k0: float>
  VOP_FMOD2K,   // fmod2k <ra: float> <rb: float> <k0: float>
  VOP_FMOD1X,   // fmod1x <ra: float> <rb: int>
  VOP_FMOD2X,   // fmod2x <ra: float> <rb: int> <rc: float>
  VOP_FMOD1XK,  // fmod1xk <ra: float> <k0: int>
  VOP_FMOD2XK,  // fmod2xk <ra: float> <rb: int> <k0: int>
  VOP_FMOD1XY,  // fmod1xy <ra: int> <rb: float>
  VOP_FMOD2XY,  // fmod2xy <ra: float> <rb: float> <rc: int>
  VOP_FMOD1XYK, // fmod1xyk <ra: float> <k0: int>
  VOP_FMOD2XYK, // fmod2xyk <ra: float> <rb: float> <k0: int>

  // register opcodes
  VOP_MOVE,
  VOP_XCHG,
  VOP_COPY,
  VOP_COPYREF,
  VOP_LOADINT,
  VOP_LOADFLOAT,
  VOP_LOADTRUE,
  VOP_LOADFALSE,
  VOP_NEWSTRING,
  VOP_NEWARRAY,
  VOP_NEWDICT,
  VOP_NEWTUPLE,
  VOP_NEWCLOSURE,

  // stack opcodes
  VOP_PUSH,
  VOP_PUSHK,
  VOP_GETARG,
  VOP_GETARGREF,
  VOP_SETARG,
  VOP_GETLOCAL,
  VOP_GETLOCALREF,
  VOP_SETLOCAL,
  VOP_DUPLOCAL,
  VOP_DUPLOCALREF,

  // stack-arithmetic opcodes
  VOP_PUSHIADD,    // pushiadd <ra: int> <rb: int>
  VOP_PUSHIADDK,   // pushiaddk <ra: int> <k0: int>
  VOP_PUSHFADD,    // pushfadd <ra: float> <rb: float>
  VOP_PUSHFADDK,   // pushfaddk <ra: float> <k0: float>
  VOP_PUSHFADDX,   // pushfaddx <ra: float> <rb: int>
  VOP_PUSHFADDXK,  // pushfaddxk <ra: int> <k0: float>
  VOP_PUSHISUB,    // pushisub <ra: int> <rb: int>
  VOP_PUSHISUBK,   // pushisubk <ra: int> <k0: int>
  VOP_PUSHFSUB,    // pushfsub <ra: float> <rb: float>
  VOP_PUSHFSUBK,   // pushfsubk <ra: float> <k0: float>
  VOP_PUSHFSUBX,   // pushfsubx <ra: float> <rb: int>
  VOP_PUSHFSUBXK,  // pushfsubxk <ra: int> <k0: float>
  VOP_PUSHIMUL,    // pushimul <ra: int> <rb: int>
  VOP_PUSHIMULK,   // pushimulk <ra: int> <k0: int>
  VOP_PUSHFMUL,    // pushfmul <ra: float> <rb: float>
  VOP_PUSHFMULK,   // pushfmulk <ra: float> <k0: float>
  VOP_PUSHFMULX,   // pushfmulx <ra: float> <rb: int>
  VOP_PUSHFMULXK,  // pushfmulxk <ra: int> <k0: float>
  VOP_PUSHIDIV,    // pushidiv <ra: int> <rb: int>
  VOP_PUSHIDIVK,   // pushidivk <ra: int> <k0: int>
  VOP_PUSHFDIV,    // pushfdiv <ra: float> <rb: float>
  VOP_PUSHFDIVK,   // pushfdivk <ra: float> <k0: float>
  VOP_PUSHFDIVX,   // pushfdivx <ra: float> <rb: int>
  VOP_PUSHFDIVXY,  // pushfdivxk <ra: float> <k0: int>
  VOP_PUSHFDIVXK,  // pushfdivxy <ra: int> <rb: float>
  VOP_PUSHFDIVXYK, // pushfdivxyk <ra: float> <k0: int>
  VOP_PUSHIPOW,    // pushipow <ra: int> <rb: int>
  VOP_PUSHIPOWK,   // pushipowk <ra: int> <k0: int>
  VOP_PUSHFPOW,    // pushfpow <ra: float> <rb: float>
  VOP_PUSHFPOWK,   // pushfpowk <ra: float> <k0: float>
  VOP_PUSHFPOWX,   // pushfpowx <ra: float> <rb: int>
  VOP_PUSHFPOWXK,  // pushfpowxk <ra: float> <k0: int>
  VOP_PUSHFPOWXY,  // pushfpowxy <ra: int> <rb: float>
  VOP_PUSHFPOWXYK, // pushfpowxyk <ra: float> <k0: int>
  VOP_PUSHIMOD,    // pushimod <ra: int> <rb: int>
  VOP_PUSHIMODK,   // pushimodk <ra: int> <k0: int>
  VOP_PUSHFMOD,    // pushfmod <ra: float> <rb: float>
  VOP_PUSHFMODK,   // pushfmodk <ra: float> <k0: float>
  VOP_PUSHFMODX,   // pushfmodx <ra: float> <rb: int>
  VOP_PUSHFMODXK,  // pushfmodxk <ra: float> <k0: int>
  VOP_PUSHFMODXY,  // pushfmodxy <ra: int> <rb: float>
  VOP_PUSHFMODXYK, // pushfmodxyk <ra: float> <k0: int>

  // function opcodes
  VOP_CAPTURE,
  VOP_CALL,
  VOP_PCALL,
  VOP_RET,
  VOP_RETNIL,
  VOP_RETTRUE,
  VOP_RETFALSE,
  VOP_RETK,
  VOP_GETUPV,
  VOP_GETUPVREF,
  VOP_SETUPV,
};

Opcode opcode_from_string(const char* str);
String opcode_to_string(Opcode opc);

struct Instruction {
  Opcode op = VOP_NOP;
  u16 a, b, c;
};

String instruction_format(Instruction insn);

} // namespace via

#endif
