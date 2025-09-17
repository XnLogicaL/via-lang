/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "defs.h"

via::Def* via::Def::from(Allocator& alloc, const ir::Stmt* node)
{
    if TRY_COERCE (const ir::StmtFuncDecl, fn, node) {
        auto* fndef = alloc.emplace<FunctionDef>();
        fndef->kind = ImplKind::SOURCE;
        fndef->code.source = fn;
        fndef->symbol = fn->symbol;
        return fndef;
    }

    return nullptr;
}

via::Def* via::Def::function(
    Allocator& alloc,
    const NativeCallback callback,
    const sema::Type* return_type,
    std::initializer_list<DefParm> parameters
)
{
    auto* fndef = alloc.emplace<FunctionDef>();
    fndef->kind = ImplKind::NATIVE;
    fndef->code.native = callback;
    fndef->parms = parameters;
    fndef->ret = return_type;

    return fndef;
}

std::string via::FunctionDef::get_dump() const
{
    return std::format(
        "FunctionDef(symbol={}, ret={}, parms={}, kind={}, code={})",
        symbol,
        ret->get_dump(),
        debug::get_dump(
            parms,
            [](const auto& parm) { return std::format("{}: {}", parm.symbol, parm.type->get_dump()); }
        ),
        magic_enum::enum_name(kind),
        reinterpret_cast<const void*>(code.native)
    );
}
