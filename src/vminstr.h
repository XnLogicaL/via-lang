// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_INSTRUCTION_H
#define VIA_INSTRUCTION_H

#include "common.h"

namespace via {

enum Opcode : uint16_t {
  VOP_NOP,

  VOP_IADD1,   // iadd1 <ra: int> <rb: int>
  VOP_IADD2,   // iadd2 <ra: int> <rb: int> <rc: int>
  VOP_IADD1K,  // iadd1k <ra: int> <k0: int>
  VOP_IADD2K,  // iadd2k <ra: int> <rb: int> <k0: int>
  VOP_FADD1,   // fadd1 <ra: float> <rb: float>
  VOP_FADD2,   // fadd2 <ra: float> <rb: float> <rc: float>
  VOP_FADD1K,  // fadd1k <ra: float> <k0: float>
  VOP_FADD2K,  // fadd2k <ra: float> <rb: float> <k0: float>
  VOP_FADD1X,  // fadd1x <ra: float> <rb: int>
  VOP_FADD2X,  // fadd2x <ra: float> <rb: int> <rc: float>
  VOP_FADD1XK, // fadd1xk <ra: int> <k0: float>
  VOP_FADD2XK, // fadd2xk <ra: float> <rb: int> <k0: float>

  VOP_ISUB1,   // isub1 <ra: int> <rb: int>
  VOP_ISUB2,   // isub2 <ra: int> <rb: int> <rc: int>
  VOP_ISUB1K,  // isub1k <ra: int> <k0: int>
  VOP_ISUB2K,  // isub2k <ra: int> <rb: int> <k0: int>
  VOP_FSUB1,   // fsub1 <ra: float> <rb: float>
  VOP_FSUB2,   // fsub2 <ra: float> <rb: float> <rc: float>
  VOP_FSUB1K,  // fsub1k <ra: float> <k0: float>
  VOP_FSUB2K,  // fsub2k <ra: float> <rb: float> <k0: float>
  VOP_FSUB1X,  // fsub1x <ra: float> <rb: int>
  VOP_FSUB2X,  // fsub2x <ra: float> <rb: int> <rc: float>
  VOP_FSUB1XK, // fsub1xk <ra: int> <k0: float>
  VOP_FSUB2XK, // fsub2xk <ra: float> <rb: int> <k0: float>

  VOP_IMUL1,   // imul1 <ra: int> <rb: int>
  VOP_IMUL2,   // imul2 <ra: int> <rb: int> <rc: int>
  VOP_IMUL1K,  // imul1k <ra: int> <k0: int>
  VOP_IMUL2K,  // imul2k <ra: int> <rb: int> <k0: int>
  VOP_FMUL1,   // fmul1 <ra: float> <rb: float>
  VOP_FMUL2,   // fmul2 <ra: float> <rb: float> <rc: float>
  VOP_FMUL1K,  // fmul1k <ra: float> <k0: float>
  VOP_FMUL2K,  // fmul2k <ra: float> <rb: float> <k0: float>
  VOP_FMUL1X,  // fmul1x <ra: float> <rb: int>
  VOP_FMUL2X,  // fmul2x <ra: float> <rb: int> <rc: float>
  VOP_FMUL1XK, // fmul1xk <ra: int> <k0: float>
  VOP_FMUL2XK, // fmul2xk <ra: float> <rb: int> <k0: float>

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
};

struct InstructionData {
  const char* comment = NULL;
};

struct Instruction {
  Opcode op = VOP_NOP;
  uint16_t a, b, c;
};

} // namespace via

#endif
