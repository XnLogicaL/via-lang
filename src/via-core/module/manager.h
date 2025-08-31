// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_MODULE_MANAGER_H_
#define VIA_CORE_MODULE_MANAGER_H_

#include <via/config.h>
#include <via/types.h>
#include "module.h"

namespace via
{

class ModuleManager
{
 public:
  friend class via::Module;

 public:
  auto& getModules() { return mModules; }
  Module* getModule(fs::path name) { return mModules[name]; }
  void addModule(Module* m) { mModules[m->mPath] = m; }
  bool hasModule(fs::path name)
  {
    return mModules.find(name) != mModules.end();
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
  Vec<String> mImports;
  Map<fs::path, Module*> mModules;
};

}  // namespace via

#endif
