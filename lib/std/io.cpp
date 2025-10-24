/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <iostream>
#include <via/via.hpp>
#include "sema/types.hpp"

namespace io {

VIA_MODULE_FUNCTION(input, vm, call_info)
{
    auto str = call_info.args.at(0);

    std::string in;
    std::cout << str->string_value();
    std::cin >> in;

    char* in_str = vm->allocator().strdup(in.c_str());
    return via::ValueRef(vm, in_str);
}

VIA_MODULE_FUNCTION(print, vm, call_info)
{
    auto str = call_info.args.at(0);
    std::cout << str->string_value();
    return via::ValueRef(vm);
}

VIA_MODULE_FUNCTION(printn, vm, call_info)
{
    auto str = call_info.args.at(0);
    std::cout << str->string_value() << "\n";
    return via::ValueRef(vm);
}

} // namespace io

VIA_MODULE_ENTRY(io, manager)
{
    auto& types = manager->type_context();

    static via::DefTable table = {
        via::DefTableEntry(
            *manager,
            via::Def::function(
                *manager,
                "input",
                via::BuiltinType::instance(types, via::BuiltinKind::STRING),
                {
                    via::DefParameter(
                        *manager,
                        "__str",
                        via::BuiltinType::instance(types, via::BuiltinKind::STRING)
                    ),
                },
                io::input
            )
        ),
        via::DefTableEntry(
            *manager,
            via::Def::function(
                *manager,
                "print",
                via::BuiltinType::instance(types, via::BuiltinKind::NIL),
                {
                    via::DefParameter(
                        *manager,
                        "__str",
                        via::BuiltinType::instance(types, via::BuiltinKind::STRING)
                    ),
                },
                io::print
            )
        ),
        via::DefTableEntry(
            *manager,
            via::Def::function(
                *manager,
                "printn",
                via::BuiltinType::instance(types, via::BuiltinKind::NIL),
                {
                    via::DefParameter(
                        *manager,
                        "__str",
                        via::BuiltinType::instance(types, via::BuiltinKind::STRING)
                    ),
                },
                io::printn
            )
        ),
    };
    return via::NativeModuleInfo::create(manager->allocator(), table);
}
