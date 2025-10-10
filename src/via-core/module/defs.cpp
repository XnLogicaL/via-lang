/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "defs.hpp"
#include "manager.hpp"
#include "sema/const.hpp"

via::DefTableEntry::DefTableEntry(
    ModuleManager& manager,
    const char* name,
    const Def* def
) noexcept
    : id(manager.symbol_table().intern(name)),
      def(def)
{}

via::DefParm::DefParm(
    ModuleManager& manager,
    const char* name,
    const sema::Type* type,
    sema::ConstValue&& init
) noexcept
    : symbol(manager.symbol_table().intern(name)),
      type(type),
      value(std::move(init))
{}

via::Def* via::Def::from(ModuleManager& manager, const ir::Stmt* node)
{
    if TRY_COERCE (const ir::StmtFuncDecl, fn, node) {
        auto* fndef = manager.allocator().emplace<FunctionDef>();
        fndef->kind = ImplKind::SOURCE;
        fndef->code.source = fn;
        fndef->symbol = fn->symbol;
        return fndef;
    }

    return nullptr;
}

via::Def* via::Def::function(
    ModuleManager& manager,
    const NativeCallback callback,
    const sema::Type* ret_type,
    std::initializer_list<DefParm> parms
)
{
    auto* fndef = manager.allocator().emplace<FunctionDef>();
    fndef->kind = ImplKind::NATIVE;
    fndef->code.native = callback;
    fndef->parms = parms;
    fndef->ret = ret_type;
    return fndef;
}

std::string via::FunctionDef::to_string() const
{
    return std::format(
        "FunctionDef(symbol={}, ret={}, parms={}, kind={}, code={})",
        symbol,
        ret->to_string(),
        debug::to_string(
            parms,
            [](const auto& parm) {
                return std::format("{}: {}", parm.symbol, parm.type->to_string());
            }
        ),
        via::to_string(kind),
        reinterpret_cast<const void*>(code.native)
    );
}
