// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_HEADER_H_
#define VIA_VM_HEADER_H_

#include <via/config.h>
#include <via/types.h>
#include "convert.h"
#include "instruction.h"
#include "lexer/location.h"

namespace via {

namespace sema {

class ConstValue;

}

struct Header {
  static constexpr u32 magic = 0x2E766961;  // .via

  u64 flags;
  Vec<sema::ConstValue> consts;
  Vec<Instruction> bytecode;

  Header() = default;
  Header(const Vec<char>& file);
};

template <>
struct Convert<Header> {
  static String to_string(const Header& header) {
    return Convert<Vec<Instruction>>::to_string(header.bytecode);
  }
};

}  // namespace via

#endif
