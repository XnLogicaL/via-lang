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
#include "module.h"
#include "module/symbol.h"
#include "sema/type_context.h"

namespace via {

class ModuleManager
{
  public:
    friend class Module;

  public:
    inline auto& get_allocator() { return m_alloc; }
    inline auto& get_type_context() { return m_type_ctx; }
    inline auto& get_symbol_table() { return m_symbol_table; }
    inline const auto& get_import_paths() const { return m_import_paths; }

    inline Module* get_module(fs::path name) { return m_modules[name]; }
    inline void push_module(Module* module) { m_modules[module->m_path] = module; }
    inline bool has_module(fs::path name)
    {
        return m_modules.find(name) != m_modules.end();
    }

    inline void push_import_path(fs::path path) { m_import_paths.push_back(path); }

    inline Module* get_module_by_name(std::string name)
    {
        for (const auto& [_, module]: m_modules) {
            if (module->m_name == name) {
                return module;
            }
        }

        return nullptr;
    }

    inline Module* get_module_by_name(SymbolId name)
    {
        if (auto symbol = m_symbol_table.lookup(name)) {
            if (auto module = get_module_by_name(std::string(*symbol))) {
                return module;
            }
        }

        return nullptr;
    }

  protected:
    inline bool is_current_import(const std::string& name) const
    {
        return std::find(m_imports.begin(), m_imports.end(), name) != m_imports.end();
    }

    inline void push_import(const std::string& name) { m_imports.push_back(name); }
    inline void pop_import()
    {
        if (!m_imports.empty()) {
            m_imports.pop_back();
        }
    }

  private:
    ScopedAllocator m_alloc;
    std::vector<std::string> m_imports;
    std::vector<fs::path> m_import_paths;
    std::unordered_map<fs::path, Module*> m_modules;
    sema::TypeContext m_type_ctx;
    SymbolTable m_symbol_table;
};

} // namespace via
