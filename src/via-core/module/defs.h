/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "ir/ir.h"
#include "sema/type.h"
#include "vm/closure.h"

namespace via {

class Module;
class ValueRef;
class CallInfo;
struct Def;

enum class ImplKind
{
    SOURCE,
    NATIVE,
};

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
};

struct DefTableEntry
{
    SymbolId id;
    const Def* def;
};

using DefTable = DefTableEntry[];

struct Def
{
    virtual SymbolId get_identity() const = 0;
    virtual std::string to_string() const = 0;

    static Def* from(ScopedAllocator& alloc, const ir::Stmt* node);
    static Def* function(
        ScopedAllocator& alloc,
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

    SymbolId get_identity() const override { return symbol; }
    std::string to_string() const override;
};

} // namespace via
