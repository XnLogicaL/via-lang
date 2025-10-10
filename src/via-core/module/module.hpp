/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <optional>
#include <parser/parser.hpp>
#include <string>
#include <via/config.hpp>
#include "defs.hpp"
#include "lexer/location.hpp"
#include "support/memory.hpp"
#include "support/os/dl.hpp"
#include "vm/executable.hpp"
#include "vm/machine.hpp"

#define VIA_MODULE_ENTRY_PREFIX viainit_

#define VIA_MODULE_ENTRY(ID, MANAGER)                                                    \
    extern "C" VIA_EXPORT const via::NativeModuleInfo*                                   \
    EXPAND_AND_PASTE(VIA_MODULE_ENTRY_PREFIX, ID)(via::ModuleManager * MANAGER)

#define VIA_MODULE_FUNCTION(ID, VM, CALL_INFO)                                           \
    via::ValueRef ID(via::VirtualMachine* VM, via::CallInfo& CALL_INFO)

namespace via {
namespace config {

VIA_CONSTANT const char MODULE_ENTRY_PREFIX[] = EXPAND_STRING(VIA_MODULE_ENTRY_PREFIX);

} // namespace config

struct NativeModuleInfo
{
    const size_t size;
    const DefTableEntry* begin;

    explicit NativeModuleInfo(const size_t size, const DefTableEntry* begin)
        : size(size),
          begin(begin)
    {}

    template <size_t Count>
        requires(Count > 0)
    static NativeModuleInfo*
    create(ScopedAllocator& alloc, const DefTableEntry (&table)[Count])
    {
        return alloc.emplace<NativeModuleInfo>(Count, table);
    }
};

class ModuleManager;

using NativeModuleInitCallback = NativeModuleInfo* (*) (ModuleManager*);

enum class ModuleKind : uint8_t
{
    SOURCE,
    NATIVE,
};

enum class ModulePerms : uint32_t
{
    NONE = 0,
    FREAD = 1 << 0,
    FWRITE = 1 << 1,
    NETWORK = 1 << 2,
    FFICALL = 1 << 3,
    IMPORT = 1 << 4,
    ALL = 0xFFFFFFFF,
};

enum class ModuleFlags : uint32_t
{
    NONE = 0,
    DUMP_TTREE = 1 << 0,
    DUMP_AST = 1 << 1,
    DUMP_IR = 1 << 2,
    DUMP_EXE = 1 << 3,
    DUMP_DEFTABLE = 1 << 4,
    NO_EXECUTION = 1 << 5,
    DEBUG = 1 << 6,
    ALL = 0xFFFFFFFF,
};

class Module final
{
  public:
    friend class ModuleManager;

  public:
    explicit Module(ModuleManager& manager)
        : m_manager(manager)
    {}

    static std::expected<Module*, std::string> load_source_file(
        ModuleManager& manager,
        Module* importee,
        const char* name,
        const std::filesystem::path& path,
        const ast::StmtImport* decl,
        const ModulePerms perms = ModulePerms::NONE,
        const ModuleFlags flags = ModuleFlags::NONE
    );

    static std::expected<Module*, std::string> load_native_object(
        ModuleManager& manager,
        Module* importee,
        const char* name,
        const std::filesystem::path& path,
        const ast::StmtImport* decl,
        const ModulePerms perms = ModulePerms::NONE,
        const ModuleFlags flags = ModuleFlags::NONE
    );

  public:
    auto name() const { return m_name; }
    auto kind() const { return m_kind; }
    auto& source() const { return m_source; }
    auto& allocator() { return m_alloc; }
    auto& manager() const { return m_manager; }
    auto ast_decl() const { return m_ast_decl; }

    std::optional<const Def*> lookup(SymbolId symbol);
    std::expected<Module*, std::string>
    import(const QualName& path, const ast::StmtImport* ast_decl);

    // TODO: Move these functions to the SourceBuffer abstraction
    std::string get_source_range(size_t begin, size_t end) const;
    std::string get_source_range(SourceLoc loc) const;

  protected:
    static void start_debugger(VirtualMachine& vm) noexcept;

  protected:
    ScopedAllocator m_alloc;
    ModuleKind m_kind;
    ModulePerms m_perms;
    ModuleFlags m_flags;
    std::string m_name;
    std::string m_source;
    std::filesystem::path m_path;
    IRTree m_ir;
    Executable* m_exe;
    std::vector<Module*> m_imports;
    std::unordered_map<SymbolId, const Def*> m_defs;
    Module* m_importee = nullptr;
    ModuleManager& m_manager;
    os::DynamicLibrary m_dl;
    const ast::StmtImport* m_ast_decl = nullptr;
};

} // namespace via
