/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <via/via.h>

namespace io
{

VIA_MOD_FUNC(print)
{
  via::debug::unimplemented("io::print");
}

}  // namespace io

VIA_MODINIT_FUNC(io)
{
  auto& symtab = mgr->getSymbolTable();
  auto& types = mgr->getTypeContext();
  auto& alloc = mgr->getAllocator();

  static via::DefTable dt = {
    {
      symtab.intern("print"),
      via::Def::newFunction(
        alloc, io::print, types.getBuiltin(via::sema::BuiltinType::Kind::NIL),
        {
          {
            .symbol = symtab.intern("__s"),
            .type = types.getBuiltin(via::sema::BuiltinType::Kind::STRING),
          },
        }),
    },
  };

  return via::NativeModuleInfo::construct(alloc, 1, dt);
}
