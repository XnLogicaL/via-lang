// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_HEADER_H_
#define VIA_VM_HEADER_H_

#include <via/config.h>
#include <via/types.h>
#include <filesystem>
#include "convert.h"
#include "diagnostics.h"
#include "instruction.h"

namespace via
{

namespace sema
{

class ConstValue;

}

struct Header
{
  static constexpr u32 kmagic = 0x2E766961;  // .via

  u32 magic;
  u64 flags;
  Vec<sema::ConstValue> consts;
  Vec<Instruction> bytecode;

  Header() = default;
  Header(const std::filesystem::path& binary, DiagnosticContext& diags);

  String get_dump() const;
};

template <>
struct Convert<Header>
{
  static String to_string(const Header& header) { return header.get_dump(); }
};

}  // namespace via

#endif
