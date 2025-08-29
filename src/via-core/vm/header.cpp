// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "header.h"
#include <sstream>
#include "color.h"
#include "debug.h"
#include "sema/const_value.h"

namespace fs = std::filesystem;

namespace via
{

Header::Header(const fs::path& binary, DiagContext& diags)
{
  debug::unimplemented("Header construction from binary file");
}

String Header::get_dump() const
{
  std::ostringstream oss;
  oss << apply_ansi_style("[section .text]\n", Fg::Yellow, Bg::Black,
                          Style::Underline);

  for (const Instruction& insn : bytecode) {
    oss << "  " << insn.get_dump() << "\n";
  }

  oss << apply_ansi_style("[section .data]\n", Fg::Yellow, Bg::Black,
                          Style::Underline);

  for (const sema::ConstValue& cv : consts) {
    oss << "  " << cv.get_dump() << "\n";
  }

  return oss.str();
}

}  // namespace via
