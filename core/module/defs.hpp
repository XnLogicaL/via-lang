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
#include "ir/ir.hpp"
#include "module/symbol.hpp"
#include "sema/const.hpp"
#include "sema/types.hpp"
#include "support/utility.hpp"
#include "vm/closure.hpp"

namespace via {

class Module;
class ValueRef;
struct CallInfo;
struct Def;

#define FOR_EACH_IMPL_KIND(X)                                                            \
    X(SOURCE)                                                                            \
    X(NATIVE)

enum class ImplKind
{
    FOR_EACH_IMPL_KIND(DEFINE_ENUM)
};

DEFINE_TO_STRING(ImplKind, FOR_EACH_IMPL_KIND(DEFINE_CASE_TO_STRING));

union ImplStorage {
    const ir::StmtFuncDecl* source;
    NativeCallback native;
};

struct SymbolInfo
{
    const Def* symbol;
    const Module* module;
};

struct DefParameter
{
    SymbolId symbol;
    QualType type;
    ConstValue value;

    VIA_NOINLINE explicit DefParameter(
        ModuleManager& manager,
        std::string name,
        QualType type,
        ConstValue&& init = ConstValue{}
    ) noexcept;

    std::string to_string(const SymbolTable& table) const;
};

struct DefTableEntry
{
    SymbolId id;
    const Def* def;

    VIA_NOINLINE explicit DefTableEntry(ModuleManager& manager, const Def* def) noexcept;
};

using DefTable = DefTableEntry[];

struct Def
{
    virtual ~Def() = default;
    virtual SymbolId identity() const = 0;
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

    SymbolId identity() const override { return symbol; }
    std::string signature(const SymbolTable& table) const override;
};

std::string to_string(
    const SymbolTable& table,
    const std::unordered_map<SymbolId, const Def*>& map
) noexcept;

} // namespace via
