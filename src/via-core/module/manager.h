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
  auto& get_modules() { return m_modules; }

  void add_module(Module* m) { m_modules[m->m_path] = m; }

  Module* get_module(fs::path name) { return m_modules[name]; }

  bool has_module(fs::path name)
  {
    return m_modules.find(name) != m_modules.end();
  }

  bool is_importing(const std::string& name) const
  {
    return std::find(m_import_stack.begin(), m_import_stack.end(), name) !=
           m_import_stack.end();
  }

  void push_import(const std::string& name) { m_import_stack.push_back(name); }

  void pop_import()
  {
    if (!m_import_stack.empty()) {
      m_import_stack.pop_back();
    }
  }

 private:
  Vec<String> m_import_stack;
  Map<fs::path, Module*> m_modules;
};

}  // namespace via

#endif
