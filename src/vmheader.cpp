// This file is a part of the lynx project (https://github.com/XnLogicaL/lynx)
// Licensed under GNU GPLv3.0 Copyright (C) 2025 XnLogicaL

#include "vmheader.h"
#include "heapbuf.h"
#include "vminstr.h"
#include "vmval.h"

namespace via {

static void write8(FileBuf* B, u8 data) {
  *B->cursor = data;
  ++B->cursor;
}

static void write16(FileBuf* B, u16 data) {
  std::memcpy(B->cursor, &data, 2);
  B->cursor += 2;
}

static void write32(FileBuf* B, u32 data) {
  std::memcpy(B->cursor, &data, 4);
  B->cursor += 4;
}

static void write64(FileBuf* B, u64 data) {
  std::memcpy(B->cursor, &data, 8);
  B->cursor += 8;
}

static void writevalue(FileBuf* B, const Value& val) {
  write8(B, val.kind);

  switch (val.kind) {
  case VLK_BOOLEAN:
    write8(B, val.u.b);
    break;
  case VLK_INT:
    write32(B, val.u.i);
    break;
  case VLK_FLOAT: {
    u32 data = std::bit_cast<u32>(val.u.f);
    write32(B, data);
  } break;
  default:
    break;
  }
}

static u8 read8(const FileBuf* B) {
  u8 data = *B->cursor;
  ++B->cursor;
  return data;
}

static u16 read16(const FileBuf* B) {
  u16 data;
  std::memcpy(&data, B->cursor, 2);
  B->cursor += 2;
  return data;
}

static u32 read32(const FileBuf* B) {
  u32 data;
  std::memcpy(&data, B->cursor, 4);
  B->cursor += 4;
  return data;
}

static u64 read64(const FileBuf* B) {
  u64 data;
  std::memcpy(&data, B->cursor, 8);
  B->cursor += 8;
  return data;
}

static Value readvalue(State* S, const FileBuf* B) {
  u8 kind = read8(B);

  switch (kind) {
  case VLK_NIL:
    return Value();
  case VLK_INT: {
    u32 data = read32(B);
    int val = static_cast<int>(data);
    return value_new(S, val);
  }
  case VLK_FLOAT: {
    u32 data = read32(B);
    float val = std::bit_cast<float>(data);
    return value_new(S, val);
  }
  case VLK_BOOLEAN: {
    u8 data = read8(B);
    bool val = static_cast<bool>(data);
    return value_new(S, val);
  }
  default:
    break;
  }

  return Value();
}

Header::Header(Header&& other)
  : magic(other.magic),
    flags(other.flags),
    consts(std::move(other.consts)),
    bytecode(std::move(other.bytecode)) {}

Header& Header::operator=(Header&& other) {
  if (this != &other) {
    magic = other.magic;
    flags = other.flags;
    consts = std::move(other.consts);
    bytecode = std::move(other.bytecode);
  }

  return *this;
}

usize header_size(const Header& H) {
  return sizeof(H.magic) + sizeof(H.flags) + H.consts.size + H.bytecode.size;
}

FileBuf header_encode(const Header& H) {
  FileBuf buf(header_size(H));

  write32(&buf, H.magic);
  write64(&buf, H.flags);
  write32(&buf, H.consts.size);

  Value* lastk = H.consts.data + H.consts.size;
  for (const Value* val = H.consts.data; val < lastk; ++val) {
    writevalue(&buf, *val);
  }

  write32(&buf, H.bytecode.size);

  Instruction* lastinsn = H.bytecode.data + H.bytecode.size;
  for (const Instruction* insn = H.bytecode.data; insn < lastinsn; ++insn) {
    u64 data = std::bit_cast<u64>(*insn);
    write64(&buf, data);
  }

  return buf;
}

Header header_decode(State* S, const FileBuf& buf) {
  buf.cursor = buf.data; // reset cursor to ensure no funny shit happens

  Header H;

  H.magic = read32(&buf);
  H.flags = read64(&buf);

  u32 kcount = read32(&buf);
  H.consts = HeapBuffer<Value>(kcount);

  for (usize i = 0; i < kcount; i++) {
    *H.consts.cursor = readvalue(S, &buf);
    ++H.consts.cursor;
  }

  u32 icount = read32(&buf);
  H.bytecode = HeapBuffer<Instruction>(icount);

  for (usize i = 0; i < icount; i++) {
    u64 data = read64(&buf);
    *H.bytecode.cursor = std::bit_cast<Instruction>(data);
    ++H.bytecode.cursor;
  }

  return H;
}

} // namespace via
