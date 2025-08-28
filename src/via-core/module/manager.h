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
  void add_module(Module* m) { m_modules[m->m_name] = m; }
  const auto& get_modules() const { return m_modules; }

 private:
  Map<String, Module*> m_modules;
};

}  // namespace via

#endif
