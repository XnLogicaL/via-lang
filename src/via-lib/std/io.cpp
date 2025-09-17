/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <via/via.h>

namespace io {

VIA_MODULE_FUNCTION(print)
{
    via::debug::unimplemented("io::print");
}

} // namespace io

VIA_MODULE_ENTRY(io)
{
    auto& symtab = mgr->get_symbol_table();
    auto& types = mgr->get_type_context();
    auto& alloc = mgr->get_allocator();

    static via::DefTable dt = {
        {
            symtab.intern("print"),
            via::Def::function(
                alloc,
                io::print,
                types.get_builtin(via::sema::BuiltinType::Kind::NIL),
                {
                    {
                        .symbol = symtab.intern("__s"),
                        .type = types.get_builtin(via::sema::BuiltinType::Kind::STRING),
                    },
                }
            ),
        },
    };

    return via::NativeModuleInfo::construct(alloc, 1, dt);
}
