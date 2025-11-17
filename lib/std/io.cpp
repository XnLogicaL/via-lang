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
    auto& alloc = vm->allocator();
    auto string = call_info.args.at(0);

    std::string in;
    std::cout << string->string_value();
    std::cin >> in;

    char* cstring = alloc.strdup(in.c_str());
    return via::ValueRef(vm, cstring);
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
    auto& symbol = manager->symbol_table();

    static via::DefTable table = {
        via::Def::function(
            *manager,
            "input",
            via::BuiltinType::instance(types, via::BuiltinKind::STRING),
            {
                {symbol.intern("__str"),
                 via::BuiltinType::instance(types, via::BuiltinKind::STRING)},
            },
            io::input
        ),
        via::Def::function(
            *manager,
            "print",
            via::BuiltinType::instance(types, via::BuiltinKind::NIL),
            {
                {symbol.intern("__str"),
                 via::BuiltinType::instance(types, via::BuiltinKind::STRING)},
            },
            io::print
        ),
        via::Def::function(
            *manager,
            "printn",
            via::BuiltinType::instance(types, via::BuiltinKind::NIL),
            {
                {symbol.intern("__str"),
                 via::BuiltinType::instance(types, via::BuiltinKind::STRING)},
            },
            io::printn
        ),
    };

    return via::NativeModuleInfo::create(manager->allocator(), table);
}
