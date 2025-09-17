/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <parser/parser.h>
#include <via/config.h>
#include <via/types.h>
#include "defs.h"
#include "support/expected.h"
#include "support/option.h"
#include "vm/executable.h"

#define VIA_MODULE_ENTRY(id) VIA_EXPORT const via::NativeModuleInfo* viainit_##id(via::ModuleManager* mgr)
#define VIA_MODULE_FUNCTION(id) via::ValueRef id(via::CallInfo& ci)

namespace via {
namespace config {

CONSTANT const char MODULE_ENTRY_PREFIX[] = "viainit_";

}

struct NativeModuleInfo
{
    const usize size;
    const DefTableEntry* begin;

    explicit NativeModuleInfo(const usize size, const DefTableEntry* begin) :
        size(size),
        begin(begin)
    {}

    static NativeModuleInfo* construct(Allocator& alloc, usize size, const DefTableEntry* begin)
    {
        return alloc.emplace<NativeModuleInfo>(size, begin);
    }
};

class ModuleManager;

using NativeModuleInitCallback = NativeModuleInfo* (*) (ModuleManager* mgr);

class Module final
{
  public:
    friend class ModuleManager;

    enum class Kind : u8
    {
        SOURCE,
        NATIVE,
    };

    enum Perms : u32
    {
        FREAD = 1 << 0,
        FWRITE = 1 << 1,
        NETWORK = 1 << 2,
        FFICALL = 1 << 3,
        IMPORT = 1 << 4,
        ALL,
    };

    enum Flags : u32
    {
        DUMP_TTREE = 1 << 0,
        DUMP_AST = 1 << 1,
        DUMP_IR = 1 << 2,
        DUMP_EXE = 1 << 3,
        DUMP_DEFTABLE = 1 << 4,
    };

  public:
    static Expected<Module*> load_source_file(
        ModuleManager* manager,
        Module* importee,
        const char* name,
        const fs::path& path,
        const ast::StmtImport* decl,
        const u32 perms = 0,
        const u32 flags = 0
    );

    static Expected<Module*> load_native_object(
        ModuleManager* manager,
        Module* importee,
        const char* name,
        const fs::path& path,
        const ast::StmtImport* decl,
        const u32 perms = 0,
        const u32 flags = 0
    );

  public:
    auto name() const { return m_name; }
    auto kind() const { return m_kind; }
    auto& get_allocator() { return m_alloc; }
    auto* get_manager() const { return m_manager; }
    auto* get_ast_decl() const { return m_ast_decl; }

    Option<const Def*> lookup(SymbolId symbol);
    Expected<Module*> import(const QualName& path, const ast::StmtImport* importDecl);

  protected:
    Allocator m_alloc;
    Kind m_kind;
    u32 m_perms, m_flags;
    std::string m_name;
    fs::path m_path;
    IRTree m_ir;
    Executable* m_exe;
    std::vector<Module*> m_imports;
    std::unordered_map<SymbolId, const Def*> m_defs;
    Module* m_importee = nullptr;
    ModuleManager* m_manager = nullptr;
    const ast::StmtImport* m_ast_decl = nullptr;
};
} // namespace via