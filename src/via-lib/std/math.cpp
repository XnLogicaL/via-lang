/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <cmath>
#include <via/via.h>

using via::ValueRef;
using namespace via::types;

namespace math {

VIA_MODULE_FUNCTION(sin, vm, call_info)
{
    auto x = call_info.args.at(0);
    auto result = std::sin(x->float_value());
    return ValueRef(vm, result);
}

} // namespace math

VIA_MODULE_ENTRY(math, manager)
{
    using via::Def;
    using via::DefTable;
    using via::NativeModuleInfo;
    using enum via::sema::BuiltinKind;

    auto& symbols = manager->get_symbol_table();
    auto& types = manager->get_type_context();
    auto& alloc = manager->get_allocator();

    static DefTable table = {
        {
            symbols.intern("sin"),
            Def::function(
                alloc,
                math::sin,
                types.get_builtin(FLOAT),
                {
                    {symbols.intern("__x"), types.get_builtin(FLOAT)},
                }
            ),
        },
    };

    return NativeModuleInfo::create(alloc, table);
}
