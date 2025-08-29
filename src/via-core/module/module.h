// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_MODULE_H_
#define VIA_CORE_MODULE_H_

#include <bytecode/builder.h>
#include <lexer/lexer.h>
#include <parser/parser.h>
#include <via/config.h>
#include <via/types.h>
#include "defs.h"

namespace via
{

class ModuleManager;
class Module final
{
 public:
  friend class ModuleManager;

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
    DUMP_BYTECODE = 1 << 3,
  };

 public:
  static Result<Module*, String> from_source(ModuleManager* mm,
                                             Module* importee,
                                             const char* name,
                                             fs::path path,
                                             u32 perms = 0,
                                             u32 flags = 0);

 public:
  Allocator& get_allocator() { return m_alloc; }

  Optional<SymbolInfo> lookup(const QualPath& qs);

  Result<Module*, String> resolve_import(const QualPath& qs);

 protected:
  Allocator m_alloc;
  u32 m_perms, m_flags;
  String m_name;
  fs::path m_path;
  ir::IrTree m_ir;
  Vec<Module*> m_imports;
  Map<SymbolId, Def*> m_defs;
  SymbolTable m_symbols;
  Module* m_importee = nullptr;
  ModuleManager* m_manager = nullptr;
};

}  // namespace via

#endif
