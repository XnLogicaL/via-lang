// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_MODULE_MANAGER_H_
#define VIA_CORE_MODULE_MANAGER_H_

#include <via/config.h>
#include <via/types.h>
#include "module.h"
#include "sema/type_context.h"

namespace via
{

namespace config
{}  // namespace config

class ModuleManager
{
 public:
  friend class via::Module;

 public:
  auto& getAllocator() { return mAlloc; }
  auto& getModules() { return mModules; }
  auto& getTypeContext() { return mTypeCtx; }
  auto& getSymbolTable() { return mSymbolTable; }
  const auto& getImportPaths() const { return mImportPaths; }

  Module* getModule(fs::path name) { return mModules[name]; }
  void addModule(Module* m) { mModules[m->mPath] = m; }
  bool hasModule(fs::path name)
  {
    return mModules.find(name) != mModules.end();
  }

  void addImportPath(fs::path path) { mImportPaths.push_back(path); }

  Module* getModuleByName(std::string name)
  {
    for (const auto& [_, module] : mModules) {
      if (module->mName == name) {
        return module;
      }
    }

    return nullptr;
  }

 protected:
  bool isImporting(const std::string& name) const
  {
    return std::find(mImports.begin(), mImports.end(), name) != mImports.end();
  }

  void pushImport(const std::string& name) { mImports.push_back(name); }
  void popImport()
  {
    if (!mImports.empty()) {
      mImports.pop_back();
    }
  }

 private:
  Allocator mAlloc;
  Vec<std::string> mImports;
  Vec<fs::path> mImportPaths;
  Map<fs::path, Module*> mModules;
  sema::TypeContext mTypeCtx;
  SymbolTable mSymbolTable;
};

}  // namespace via

#endif
