/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <cmath>
#include <via/via.hpp>
#include "sema/types.hpp"

namespace math {

VIA_MODULE_FUNCTION(sin, vm, call_info)
{
    auto x = call_info.args.at(0);
    auto result = std::sin(x->float_value());
    return via::ValueRef(vm, result);
}

} // namespace math

VIA_MODULE_ENTRY(math, manager)
{
    auto& types = manager->type_context();

    static via::DefTable table = {
        via::DefTableEntry(
            *manager,
            via::Def::function(
                *manager,
                "sin",
                via::BuiltinType::instance(types, via::BuiltinKind::FLOAT),
                {
                    via::DefParameter(
                        *manager,
                        "__x",
                        via::BuiltinType::instance(types, via::BuiltinKind::FLOAT)
                    ),
                },
                math::sin
            )
        ),
    };

    return via::NativeModuleInfo::create(manager->allocator(), table);
}
