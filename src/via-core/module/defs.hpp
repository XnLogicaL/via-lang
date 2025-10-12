/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.hpp>
#include "ir/ir.hpp"
#include "module/symbol.hpp"
#include "sema/const.hpp"
#include "sema/type.hpp"
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

struct DefParm
{
    SymbolId symbol;
    const sema::Type* type;
    sema::ConstValue value;

    VIA_NOINLINE explicit DefParm(
        ModuleManager& manager,
        const char* name,
        const sema::Type* type,
        sema::ConstValue&& init = sema::ConstValue{}
    ) noexcept;
};

struct DefTableEntry
{
    SymbolId id;
    const Def* def;

    VIA_NOINLINE explicit DefTableEntry(
        ModuleManager& manager,
        const char* name,
        const Def* def
    ) noexcept;
};

using DefTable = DefTableEntry[];

struct Def
{
    virtual SymbolId identity() const = 0;
    virtual std::string to_string() const = 0;

    VIA_NOINLINE static Def* from(ModuleManager& manager, const ir::Stmt* node);
    VIA_NOINLINE static Def* function(
        ModuleManager& manager,
        const NativeCallback callback,
        const sema::Type* ret_type,
        std::initializer_list<DefParm> parms
    );
};

struct FunctionDef: public Def
{
    ImplKind kind;
    ImplStorage code;
    SymbolId symbol;
    std::vector<DefParm> parms;
    const sema::Type* ret;

    SymbolId identity() const override { return symbol; }
    std::string to_string() const override;
};

} // namespace via
