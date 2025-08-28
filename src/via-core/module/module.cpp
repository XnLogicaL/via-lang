// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "module.h"
#include <filesystem>
#include <fstream>
#include "ir/builder.h"
#include "manager.h"
#include "sema/register.h"
#include "sema/stack.h"

#ifdef VIA_PLATFORM_UNIX
#include <dlfcn.h>
#elif defined(VIA_PLATFORM_WINDOWS)
#include <windows.h>
#endif

namespace via
{

namespace fs = std::filesystem;

static Allocator module_alloc;

static Result<String, String> read_file(const fs::path& path)
{
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    return std::unexpected(
        fmt::format("no such file or directory: '{}'", path.string()));
  }

  via::String line;
  via::String content;
  while (std::getline(ifs, line)) {
    content += line;
    content += '\n';
  }

  return content;
}

namespace native
{

using ModuleInitFunc = const Module* (*)(ModuleManager* mm);

#ifdef VIA_PLATFORM_WINDOWS
struct WinLibraryManager
{
  Vec<HMODULE> libs;

  ~WinLibraryManager()
  {
    for (HMODULE handle : libs) {
      FreeLibrary(handle);
    }
  }

  void add(HMODULE h) { libs.push_back(h); }
};

static WinLibraryManager win_libs;
#endif

static ModuleInitFunc load_symbol(const fs::path& path, const char* symbol)
{
#ifdef VIA_PLATFORM_UNIX
  void* handle = dlopen(path.c_str(), RTLD_NOW);
  if (!handle)
    return nullptr;
  void* sym = dlsym(handle, symbol);
  return reinterpret_cast<ModuleInitFunc>(sym);
#elif defined(VIA_PLATFORM_WINDOWS)
  // LoadLibrary accepts LPCWSTR or LPCSTR depending on unicode macros; we
  // assume ANSI path.
  HMODULE handle = LoadLibraryA(path.string().c_str());
  if (!handle)
    return nullptr;
  win_libs.add(handle);
  FARPROC proc = GetProcAddress(handle, symbol);
  return reinterpret_cast<ModuleInitFunc>(proc);
#else
  (void)path;
  (void)symbol;
  return nullptr;
#endif
}

static String get_symbol(const char* name)
{
  return fmt::format("viainit_{}", name);
}

}  // namespace native

[[nodiscard]] Module* Module::from_source(ModuleManager* mm,
                                          const char* name,
                                          fs::path path,
                                          u32 perms,
                                          u32 flags)
{
  auto file = read_file(path);
  if (!file.has_value()) {
    spdlog::error(file.error());
    goto error;
  }

  {
    Module* m = module_alloc.emplace<Module>();
    m->m_manager = mm;
    m->m_name = name;
    m->m_path = path.string();

    mm->add_module(m);

    DiagnosticContext diags(path.string(), *file);

    Lexer lexer(*file);
    auto tt = lexer.tokenize();

    Parser parser(*file, tt, diags);
    auto ast = parser.parse();

    // reset semantic state
    sema::registers::reset();
    sema::stack::reset();

    IRBuilder irb(m, ast, diags);
    m->m_ir = irb.build();

    for (const ir::Entity* e : m->m_ir) {
      m->m_defs[e->symbol] = Def::from(m, e);
    }

    bool failed = diags.has_errors();

    diags.emit();
    diags.clear();

    if (flags & DUMP_TTREE)
      debug::dump(tt);
    if (flags & DUMP_AST)
      debug::dump(ast);

    if (failed) {
      if ((flags & (DUMP_TTREE | DUMP_AST | DUMP_IR)) != 0u) {
        spdlog::warn("dump may be invalid due to compilation failure");
      }
      goto error;
    }

    return m;
  }

error:
  return nullptr;
}

[[nodiscard]] Optional<SymbolInfo> Module::lookup(const QualPath& qs)
{
  if (m_manager == nullptr) {
    return nullopt;
  }

  const auto& modules = m_manager->get_modules();
  if (auto it = modules.find(qs[0]); it != modules.end()) {
    Module* module = it->second;
    QualPath new_qs = qs;
    new_qs.pop_front();

    SymbolId id = module->m_symbols.intern(new_qs);

    return SymbolInfo{
        .def = module->m_defs[id],
        .mod = module,
    };
  } else {
    return nullopt;
  }
}

}  // namespace via
