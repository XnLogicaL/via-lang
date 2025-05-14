// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "sema.h"
#include "visitor.h"

namespace via {

namespace sema {

register_t alloc_register(VisitorContext& ctx) {
  register_t reg = ctx.reg_alloc.allocate_register();
  if (reg == 0xFFFF) {
    error(ctx, "Register allocation failure");
    info(
      ctx,
      "This likely indicates an internal compiler bug. Please report this issue in the official "
      "via language github repository."
    );
    flush(ctx);
  }

  return reg;
}

void free_register(VisitorContext& ctx, operand_t reg) {
  ctx.reg_alloc.free_register(reg);
}

} // namespace sema

} // namespace via
