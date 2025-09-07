/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <lexer/lexer.h>
#include <parser/parser.h>
#include <via/config.h>
#include <via/types.h>
#include "defs.h"
#include "expected.h"
#include "option.h"

#define VIA_MODINIT_FUNC(id) \
  VIA_EXPORT const via::NativeModuleInfo* viainit_##id(via::ModuleManager* mgr)

#define VIA_MOD_FUNC(id) via::ValueRef id(via::CallInfo& ci)

namespace via
{

namespace config
{

inline constexpr const char kInitCallbackPrefix[] = "viainit_";

}

struct NativeModuleInfo
{
  const usize size;
  const DefTableEntry* begin;

  explicit NativeModuleInfo(usize size, const DefTableEntry* begin)
      : size(size), begin(begin)
  {}

  static NativeModuleInfo* construct(Allocator& alloc,
                                     usize size,
                                     const DefTableEntry* begin)
  {
    return alloc.emplace<NativeModuleInfo>(size, begin);
  }
};

class ModuleManager;

using NativeModuleInitCallback = NativeModuleInfo* (*)(ModuleManager* mgr);

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
    DUMP_BYTECODE = 1 << 3,
    DUMP_DEFTABLE = 1 << 4,
  };

 public:
  static Expected<Module*> loadSourceFile(ModuleManager* mgr,
                                          Module* importee,
                                          const char* name,
                                          fs::path path,
                                          u32 perms = 0,
                                          u32 flags = 0);

  static Expected<Module*> loadNativeObject(ModuleManager* mgr,
                                            Module* importee,
                                            const char* name,
                                            fs::path path,
                                            u32 perms = 0,
                                            u32 flags = 0);

 public:
  auto getKind() const { return mKind; }
  auto& getAllocator() { return mAlloc; }
  auto* getManager() { return mManager; }

  Option<SymbolInfo> lookup(const QualPath& path);
  Expected<Module*> resolveImport(const QualPath& path);

  std::string dump() const;

 protected:
  Allocator mAlloc;
  Kind mKind;
  u32 mPerms, mFlags;
  std::string mName;
  fs::path mPath;
  IRTree mIr;
  Vec<Module*> mImports;
  Map<SymbolId, const Def*> mDefs;
  Module* mImportee = nullptr;
  ModuleManager* mManager = nullptr;
};

}  // namespace via
