// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_HEADER_H_
#define VIA_VM_HEADER_H_

#include <via/config.h>
#include <via/types.h>
#include "instruction.h"
#include "lexer/location.h"

namespace via {

struct Value;
struct Header {
  static constexpr u32 magic = 0x2E766961;  // .via

  u64 flags;
  Vec<Value> ks;        // constants
  Vec<Instruction> is;  // Instructions

  Header() = default;
  Header(const FileBuf& buf);
};

}  // namespace via

#endif
