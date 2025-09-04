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
#include "module/symbol.h"

#define VIA_MODINIT_FUNC(id) \
  VIA_EXPORT const via::NativeModuleInfo* viainit_##id(via::ModuleManager* mgr)

#define VIA_MOD_FUNC(id) via::ValueRef id(via::CallInfo& ci)

namespace via
{

struct NativeModuleInfo
{
  const usize size;
  const DefTableEntry* dt;

  static NativeModuleInfo* construct(Allocator& alloc,
                                     usize sz,
                                     const DefTableEntry* dt)
  {
    return alloc.emplace<NativeModuleInfo>(sz, dt);
  }

  NativeModuleInfo(usize size, const DefTableEntry* dt) : size(size), dt(dt) {}
};

class ModuleManager;
class Module final
{
 public:
  friend class ModuleManager;

  using InitCallback = NativeModuleInfo* (*)(ModuleManager* mgr);

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
    DUMP_BYTECODE = 1 << 3,
    DUMP_DEFTABLE = 1 << 4,
  };

 public:
  static Result<Module*, String> loadSourceFile(ModuleManager* mgr,
                                                Module* importee,
                                                const char* name,
                                                fs::path path,
                                                u32 perms = 0,
                                                u32 flags = 0);

  static Result<Module*, String> loadNativeObject(ModuleManager* mgr,
                                                  Module* importee,
                                                  const char* name,
                                                  fs::path path,
                                                  u32 perms = 0,
                                                  u32 flags = 0);

 public:
  String dump() const;
  Kind getKind() const { return mKind; }
  Allocator& getAllocator() { return mAlloc; }
  Optional<SymbolInfo> lookup(const QualPath& qs);
  Result<Module*, String> resolveImport(const QualPath& qs);

 protected:
  Allocator mAlloc;
  Kind mKind;
  u32 mPerms, mFlags;
  String mName;
  fs::path mPath;
  IrTree mIr;
  Vec<Module*> mImports;
  Map<SymbolId, const Def*> mDefs;
  Module* mImportee = nullptr;
  ModuleManager* mManager = nullptr;
};

}  // namespace via

#endif
