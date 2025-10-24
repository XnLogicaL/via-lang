/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "defs.hpp"
#include <sstream>
#include "manager.hpp"
#include "module/symbol.hpp"
#include "sema/const.hpp"
#include "sema/types.hpp"
#include "support/ansi.hpp"

VIA_NOINLINE
via::DefTableEntry::DefTableEntry(ModuleManager& manager, const Def* def) noexcept
    : id(def->identity()),
      def(def)
{}

VIA_NOINLINE via::DefParameter::DefParameter(
    ModuleManager& manager,
    std::string name,
    QualType type,
    ConstValue&& init
) noexcept
    : symbol(manager.symbol_table().intern(name)),
      type(type),
      value(std::move(init))
{}

std::string via::DefParameter::to_string(const SymbolTable& table) const
{
    return std::format(
        "{}: {}",
        table.lookup(symbol).value_or("<symbol error>"),
        type.to_string()
    );
}

VIA_NOINLINE via::Def* via::Def::from(ModuleManager& manager, const ir::Stmt* node)
{
    if TRY_COERCE (const ir::StmtFuncDecl, decl, node) {
        auto* function = manager.allocator().emplace<FunctionDef>();
        function->kind = ImplKind::SOURCE;
        function->code.source = decl;
        function->ret = decl->ret;
        function->symbol = decl->symbol;

        for (const auto& parm: decl->parms) {
            function->parms.emplace_back(
                manager,
                std::string(
                    manager.symbol_table().lookup(parm.symbol).value_or("<parameter>")
                ),
                parm.type
            );
        }
        return function;
    }
    return nullptr;
}

VIA_NOINLINE via::Def* via::Def::function(
    ModuleManager& manager,
    std::string name,
    QualType ret,
    std::initializer_list<DefParameter> parms,
    const NativeCallback callback
)
{
    auto* function = manager.allocator().emplace<FunctionDef>();
    function->kind = ImplKind::NATIVE;
    function->code.native = callback;
    function->parms = parms;
    function->ret = ret;
    function->symbol = manager.symbol_table().intern(name);
    return function;
}

std::string via::FunctionDef::signature(const SymbolTable& table) const
{
    return std::format(
        "fn {} {} -> {}",
        table.lookup(symbol).value_or("<symbol error>"),
        via::to_string(
            parms,
            [&table](const auto& parm) { return parm.to_string(table); },
            "(",
            ")"
        ),
        ret.to_string()
    );
}

std::string via::to_string(
    const SymbolTable& table,
    const std::unordered_map<SymbolId, const Def*>& map
) noexcept
{
    std::ostringstream oss;
    oss << ansi::format(
        "[disassembly of def table]:\n",
        ansi::Foreground::YELLOW,
        ansi::Background::NONE,
        ansi::Style::UNDERLINE
    );

    oss << ansi::format(
        "  id    kind        signature           \n"
        "  ----  ----------  --------------------\n",
        ansi::Foreground::NONE,
        ansi::Background::NONE,
        ansi::Style::FAINT
    );

    for (size_t i = 0; const auto& it: map) {
        oss << "  "
            << ansi::format(
                   std::format("{:0>4}  ", i++),
                   ansi::Foreground::NONE,
                   ansi::Background::NONE,
                   ansi::Style::FAINT
               );
        if TRY_COERCE (const FunctionDef, function_def, it.second) {
            oss << "function  ";
            oss << "  " << function_def->signature(table) << "\n";
        } else {
            oss << "unknown   ";
            oss << "address: " << (void*) it.second << "\n";
        }
    }
    oss << "\n";
    return oss.str();
}
