// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

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
  using enum via::sema::BuiltinType::Kind;

  auto& symtab = mgr->getSymbolTable();
  auto& types = mgr->getTypeContext();
  auto& alloc = mgr->getAllocator();

  static via::DefTable dt = {
      {
          symtab.intern("print"),
          via::Def::newFunction(
              alloc, io::print,
              {{
                  .symbol = symtab.intern("__s"),
                  .type = types.getBuiltinTypeInstance(String),
              }},
              types.getBuiltinTypeInstance(Nil)),
      },
  };

  return via::NativeModuleInfo::construct(alloc, 1, dt);
}
