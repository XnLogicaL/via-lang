/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <string>
#include <via/config.hpp>
#include "sema/const.hpp"
#include "sema/types.hpp"
#include "symbol.hpp"
#include "vm/closure.hpp"

namespace via {

union ImplStorage {
    const ir::StmtFuncDecl* source;
    NativeCallback native;
};

class Module;
class ValueRef;
struct CallInfo;
struct Def;

struct SymbolInfo
{
    const Def* symbol;
    const Module* module;
};

struct DefParameter
{
    SymbolId symbol;
    QualType type;
    ConstValue value{};

    std::string to_string(const SymbolTable& table) const;
};

using DefTable = const Def*[];

struct Def
{
    virtual ~Def() = default;
    virtual std::optional<SymbolId> identity() const = 0;
    virtual std::string signature(const SymbolTable&) const { return "<no identity>"; }

    VIA_NOINLINE static Def* from(ModuleManager& manager, const ir::Stmt* node);
    VIA_NOINLINE static Def* function(
        ModuleManager& manager,
        std::string name,
        QualType ret,
        std::initializer_list<DefParameter> parms,
        NativeCallback callback
    );
};

struct FunctionDef: public Def
{
    ImplKind kind;
    ImplStorage code;
    SymbolId symbol;
    QualType ret;
    std::vector<DefParameter> parms;

    std::optional<SymbolId> identity() const override { return symbol; }
    std::string signature(const SymbolTable& table) const override;
};

std::string to_string(
    const SymbolTable& table,
    const std::unordered_map<SymbolId, const Def*>& map
) noexcept;

} // namespace via
