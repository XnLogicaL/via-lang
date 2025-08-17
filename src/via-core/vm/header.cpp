// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "header.h"
#include <sstream>
#include "color.h"

namespace via {

String Header::get_dump() const {
  std::ostringstream oss;
  oss << apply_color("[section .text]", Fg::White, Bg::Black, Style::Underline)
      << "\n";

  for (const Instruction& insn : bytecode) {
    oss << "  " << insn.get_dump() << "\n";
  }

  return oss.str();
}

}  // namespace via
