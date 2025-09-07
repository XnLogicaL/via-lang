/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "type.h"
#include "ir/ir.h"

namespace sema = via::sema;

via::Expected<sema::Type*> sema::Type::infer(Allocator& alloc,
                                             const ast::Expr* expr)
{
  return nullptr;
}

via::Expected<sema::Type*> sema::Type::from(Allocator& alloc,
                                            const ast::Type* type)
{
  return nullptr;
}
