/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <filesystem>
#include <via/config.hpp>
#include "module.hpp"
#include "sema/context.hpp"
#include "symbol.hpp"

namespace via {

class ModuleManager
{
  public:
    friend class Module;

  public:
    auto& allocator() { return m_alloc; }
    auto& type_context() { return m_type_ctx; }
    auto& symbol_table() { return m_symbol_table; }
    auto get_import_paths() const { return m_import_paths; }

    void push_module(Module* module) { m_modules[module->m_path] = module; }
    auto* get_module(std::filesystem::path name) { return m_modules[name]; }
    bool has_module(std::filesystem::path name)
    {
        return m_modules.find(name) != m_modules.end();
    }

    void push_import_path(std::filesystem::path path) { m_import_paths.push_back(path); }
    Module* get_module_by_name(std::string name)
    {
        for (const auto& [_, module]: m_modules) {
            if (module->m_name == name) {
                return module;
            }
        }
        return nullptr;
    }

    Module* get_module_by_name(SymbolId name)
    {
        if (auto symbol = m_symbol_table.lookup(name)) {
            if (auto module = get_module_by_name(std::string(*symbol))) {
                return module;
            }
        }

        return nullptr;
    }

  protected:
    bool is_current_import(const std::string& name) const
    {
        return std::find(m_imports.begin(), m_imports.end(), name) != m_imports.end();
    }

    void push_import(const std::string& name) { m_imports.push_back(name); }
    void pop_import()
    {
        if (!m_imports.empty()) {
            m_imports.pop_back();
        }
    }

  private:
    ScopedAllocator m_alloc;
    SymbolTable m_symbol_table;
    sema::TypeContext m_type_ctx;
    std::vector<std::string> m_imports;
    std::vector<std::filesystem::path> m_import_paths;
    std::unordered_map<std::filesystem::path, Module*> m_modules;
};

} // namespace via
