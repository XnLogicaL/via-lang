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
#include "module/defs.h"

using via::ValueRef;

namespace io {

VIA_MODULE_FUNCTION(print, vm, call_info)
{
    auto str = call_info.args.at(0);
    std::cout << str->string_value();
    return ValueRef(vm);
}

VIA_MODULE_FUNCTION(printn, vm, call_info)
{
    auto str = call_info.args.at(0);
    std::cout << str->string_value() << "\n";
    return ValueRef(vm);
}

} // namespace io

VIA_MODULE_ENTRY(io, manager)
{
    using enum via::sema::BuiltinKind;

    auto& types = manager->type_context();

    static via::DefTable table = {
        via::DefTableEntry(
            *manager,
            "print",
            via::Def::function(
                *manager,
                io::print,
                types.get_builtin(NIL),
                {
                    via::DefParm(*manager, "__str", types.get_builtin(STRING)),
                }
            )
        ),
        via::DefTableEntry(
            *manager,
            "printn",
            via::Def::function(
                *manager,
                io::printn,
                types.get_builtin(NIL),
                {
                    via::DefParm(*manager, "__str", types.get_builtin(STRING)),
                }
            )
        ),
    };

    return via::NativeModuleInfo::create(manager->allocator(), table);
}
