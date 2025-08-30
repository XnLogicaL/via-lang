// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ir.h"

namespace via
{

namespace ir
{}

namespace debug
{

[[nodiscard]] String dump(const IrTree& ir)
{
  std::ostringstream oss;

  for (const ir::Entity* e : ir) {
    oss << e->dump() << "\n";
  }

  return oss.str();
}

}  // namespace debug

}  // namespace via
