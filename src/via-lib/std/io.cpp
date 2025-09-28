/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <iostream>
#include <via/via.h>

namespace io {

VIA_MODULE_FUNCTION(print, vm, ci)
{
    std::cout << "Hello from C++!\n";

    for (const auto& arg: ci.args) {
        std::cout << (const void*) arg.get() << "\n";
    }

    return via::ValueRef(vm);
}

} // namespace io

VIA_MODULE_ENTRY(io, mgr)
{
    auto& symtab = mgr->get_symbol_table();
    auto& types = mgr->get_type_context();
    auto& alloc = mgr->get_allocator();

    static via::DefTable dt = {
        {
            .id = symtab.intern("print"),
            .def = via::Def::function(
                alloc,
                io::print,
                types.get_builtin(via::sema::BuiltinKind::NIL),
                {{
                    .symbol = symtab.intern("__s"),
                    .type = types.get_builtin(via::sema::BuiltinKind::STRING),
                }}
            ),
        },
    };

    return via::NativeModuleInfo::create(alloc, 1, dt);
}
