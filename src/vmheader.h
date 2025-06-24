// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HEADER_H
#define VIA_HEADER_H

#include <cstdint>

#include "heapbuf.h"
#include "vminstr.h"
#include "vmval.h"

namespace via {

namespace vm {

struct Header {
  uint32_t magic = 0xDEADCAFE;
  uint64_t flags;

  HeapBuffer<Value> consts;
  HeapBuffer<Instruction> bytecode;

  Header() = default;

  VIA_NOCOPY(Header);
  VIA_IMPLMOVE(Header);
};

using FileBuf = HeapBuffer<uint8_t>;

size_t header_size(const Header& H);
FileBuf header_encode(const Header& H);
Header header_decode(const FileBuf& buf);

} // namespace vm

} // namespace via

#endif